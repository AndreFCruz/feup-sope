#include "Request.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>


/*
#ifndef DEBUG
#define DEBUG
#endif
*/

#define SHARED 0
#define MAX_THREADS 1000

int out_fifo, in_fifo;
int out_fd;
int pid;
unsigned long long time_init;

int MAX_SITS;

//Final statistic info
int received[2];
int served[2];
int rejected[2];

sem_t places_sem;

pthread_mutex_t served_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logs_mut = PTHREAD_MUTEX_INITIALIZER;

/**
* Prints a request to the register file
*/
void print_register(Request* req, const char* tip);

/**
* Gets actual clock miliseconds
*/
void get_clock(double *time);

/**
* Simulates the steam room utilization
*/
void * request_handler(void *arg);

/**
 * Simulates the reception and processing of the requests
 */
void * mainThread(void * arg);

/**
* Making and opening FIFOs and regist file
*/
void fileHandler();

/**
 * Prints overall statistics
 */
void print_final_stats();

int main(int argc, char** argv){
	if(argc!=2)
	{
		printf("Usage: %s <no_places> \n", argv[0]);
		exit(1);
	}

	MAX_SITS = atoi(argv[1]);

	//Getting the time of init
	time_init=get_current_time();

	//Semaphore intializer
	if (sem_init(&places_sem, SHARED, MAX_SITS) != 0)
		perror("sem_init failed");

	fileHandler();

	pthread_t tid1;
	pthread_create(&tid1, NULL, mainThread, NULL);

	// Wait for mainThread to terminate
	pthread_join(tid1, NULL);

	// Print overall statistics
	print_final_stats();

	// Closing files and deleting created FIFOs
	close(in_fifo);
	close(out_fifo);

	exit(0);
}

void * request_handler(void *arg){
	Request * req = (Request *) arg;

#ifdef DEBUG
	printf("bal.handler: starting %d\n", request_get_serial_no(req));
#endif

	pthread_mutex_lock( &served_mut );
	served[((size_t) request_get_gender(req)) % 2]++;
	pthread_mutex_unlock( &served_mut );

	usleep(request_get_duration(req) * MILI_TO_MICRO);

	print_register(req, MSG_SERVED);

	// Signal an empty seat
	sem_post(&places_sem);

#ifdef DEBUG
	printf("bal.handler: exiting %d\n", request_get_serial_no(req));
#endif

	pthread_exit(req);
}

void fileHandler(){
	if (mkfifo(REQUESTS_FIFO_PATH, FILE_PERMISSIONS) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}
	if (mkfifo(REJECTED_FIFO_PATH, FILE_PERMISSIONS) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}

	if ((in_fifo=open(REQUESTS_FIFO_PATH,O_RDONLY)) == -1) {
		perror("Can't open FIFO REQUESTS_FIFO_PATH\n");
		exit(5);
	}

	if ((out_fifo=open(REJECTED_FIFO_PATH,O_WRONLY)) == -1) {
		perror("Can't open FIFO FIFO_REJECTED\n");
		exit(4);
	}

	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "/tmp/bal.%d", (int) getpid());

	if((out_fd=open(filename, O_RDWR|O_CREAT, FILE_PERMISSIONS)) == -1) {
		perror("Can't open LOGS FILE");
		exit(6);
	}
}

void * mainThread(void * arg){
#ifdef DEBUG
	printf("bal.mainThread: starting\n");
#endif

	const ssize_t SIZEOF_REQUEST = request_get_sizeof();
	
	char gender = '*';

	int i = 0;
	Request * req = malloc(SIZEOF_REQUEST);
	pthread_t threads[MAX_THREADS];

	while (read(in_fifo, req, SIZEOF_REQUEST) > 0)
	{

#ifdef DEBUG
		printf("bal.mainThread: in while -- ");
#endif

		// Check if sauna is empty
		int num = 0;
		if (sem_getvalue(&places_sem, &num) != 0)
			perror("sem_getvalue failed");

		if(num == MAX_SITS) {
			printf("\n\n**RESET SAUNA'S GENDER **\n\n");

			gender = '*';
		}

		// Check if sauna has empty seats
		sem_wait(&places_sem);

		// Log Request receival
		print_register(req, MSG_RECEIVED);
		received[((size_t) request_get_gender(req)) % 2]++;

		if (gender == '*')
			gender = request_get_gender(req);

		if (gender == request_get_gender(req)) { // Accepted
			write (out_fifo, &SIGNAL_CHAR, sizeof(char)); // Signal request accepted

			Request * tmp_req = malloc(SIZEOF_REQUEST);
			memcpy(tmp_req, req, SIZEOF_REQUEST);

#ifdef DEBUG
			printf("ACCEPT\n");
#endif

			pthread_create(&threads[i++], NULL, request_handler, (void *) tmp_req);
		} else { // Rejected
			rejected[((size_t) request_get_gender(req)) % 2]++;
			print_register(req, MSG_REJECTED);
			write(out_fifo, req, SIZEOF_REQUEST);
			sem_post(&places_sem);

#ifdef DEBUG
			printf("REJECT\n");
#endif
		}
	}

#ifdef DEBUG
	printf("bal.mainThread: joining with handler threads and freeing memory\n");
#endif

	int j; // Join with all created threads
	for(j = 0; j < i; j++) {
		Request * ptr = NULL;
		pthread_join(threads[j], (void *) ptr);

		free(ptr); // free previously allocated memory
	}

	free(req);

#ifdef DEBUG
	printf("bal.mainThread: exiting\n");
#endif

	return NULL;
}

void print_register(Request* req, const char * msg){
	pthread_mutex_lock( &logs_mut );

	unsigned long long time_elapsed = get_current_time() - time_init;

	dprintf(out_fd, "%4llu.%02llu - %4d - %#08X - %4d: %c - %4d - %s\n",
		(time_elapsed) / MILI_TO_MICRO,	/* current time instant in miliseconds */
		((time_elapsed) % (MILI_TO_MICRO) + 5) / 10,	/* rounded decimals */
		pid,						/* process pid */
		pthread_self(),				/* thread tid */
		request_get_serial_no(req),	/* request's serial number */
		request_get_gender(req),	/* request's gender */
		request_get_duration(req),	/* request's duration */
		msg);						/* message identifier */

	pthread_mutex_unlock( &logs_mut );
}

void print_final_stats() {

	printf("\nReceptions (M/F): %d/%d [%d]\n", received[1], received[0], received[1] + received[0]);
	printf("Rejections (M/F): %d/%d [%d]\n", rejected[1], rejected[0], rejected[0] + rejected[1]);
	printf("Served     (M/F): %d/%d [%d]\n", served[1], served[0], served[0] + served[1]);
	printf("\nCurrent  instant: %llu\n", get_current_time() - time_init);
	
	return;
}
