#include "Request.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>

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
	sem_init(&places_sem, SHARED, MAX_SITS);

	fileHandler();

	pthread_t tid1;
	pthread_create(&tid1, NULL, mainThread, NULL);

	// Wait for mainThread to terminate
	pthread_join(tid1, NULL);

	// Print overall statistics
	print_final_stats();

	// Closing files and deleting created FIFOs
	// TODO Check termination clauses
	close(in_fifo);
	close(out_fifo);
	unlink(REJECTED_FIFO_PATH);

	exit(0);
}

void * request_handler(void *arg){
	// Check if sauna has empty seats
	sem_wait(&places_sem);
	Request * req = (Request *) arg;

	pthread_mutex_lock( &served_mut );
	served[((size_t) request_get_gender(req)) % 2]++;
	pthread_mutex_unlock( &served_mut );

	usleep(request_get_duration(req) * MILI_TO_MICRO);

	print_register(req, MSG_SERVED);

	// Signal an empty seat
	sem_post(&places_sem);

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

	pid=(int) getpid();
	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "/tmp/bal.%d", pid);

	if((out_fd=open(filename, O_RDWR|O_CREAT, FILE_PERMISSIONS)) == -1) {
		perror("Can't open LOGS FILE");
		exit(6);
	}
}

void * mainThread(void * arg){
	const ssize_t SIZEOF_REQUEST = request_get_sizeof();
	
	char gender = '*';

	int i = 0;
	Request * req = malloc(SIZEOF_REQUEST);
	pthread_t threads[MAX_THREADS];

	while (read(in_fifo, req, SIZEOF_REQUEST) > 0)
	{
		// Check if sauna is empty
		int num = 0;
		sem_getvalue(&places_sem, &num);
		
		if(num == MAX_SITS)
			gender = '*';

		// Log Request receival
		print_register(req, MSG_RECEIVED);
		received[((size_t) request_get_gender(req)) % 2]++;

		if (gender == '*')
			gender = request_get_gender(req);

		if (gender == request_get_gender(req)) { // Accepted
			write (out_fifo, 0, 1); // Signal request accepted

			Request * tmp_req = malloc(SIZEOF_REQUEST);
			memcpy(tmp_req, req, SIZEOF_REQUEST);

			pthread_create(&threads[i++], NULL, request_handler, (void *) tmp_req);
		} else { // Rejected
			rejected[((size_t) request_get_gender(req)) % 2]++;
			print_register(req, MSG_REJECTED);
			write(out_fifo, req, SIZEOF_REQUEST);
		}
	}

	int j; // Join with all created threads
	for(j = 0; j < i; j++){
		Request ** ptr = NULL;
		pthread_join(threads[j], NULL);

		free(ptr); // free previously allocated memory
	}

	free(req);

	return NULL;
}

void print_register(Request* req, const char * msg){
	pthread_mutex_lock( &logs_mut );

	unsigned long long time_elapsed = get_current_time() - time_init;

	dprintf(out_fd, "%4llu - %4d - %10lu - %4d: %c - %4d - %s\n",
		time_elapsed,								/* current time instance in miliseconds */
		pid,										/* process pid */
		(long unsigned) pthread_self(),				/* thread tid */
		request_get_serial_no(req),					/* request's serial number */
		request_get_gender(req),					/* request's gender */
		request_get_duration(req),					/* request's duration */
		msg);										/* message identifier */

	pthread_mutex_unlock( &logs_mut );
}

void print_final_stats() {
	printf("Number of received requests:\n");
	printf("Male - %d\n", received[1]);
	printf("Female - %d\n", received[0]);
	printf("Total - %d\n", received[0]+received[1]);
	
	printf("Number of rejected requests:\n");
	printf("Male - %d\n", rejected[1]);
	printf("Female - %d\n", rejected[0]);
	printf("Total - %d\n", rejected[0]+rejected[1]);
	
	printf("Number of served requests:\n");
	printf("Male - %d\n", served[1]);
	printf("Female - %d\n", served[0]);
	printf("Total - %d\n", served[0]+served[1]);
	
	return;
}
