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

char gender;
int out_fifo, in_fifo;
int out_fd;
int pid;
unsigned long long time_init;

//Final statistic info
int received[2];
int served[2];
int rejected[2];

sem_t out_sem;
sem_t places_sem;

pthread_mutex_t gender_mut=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logs_mut=PTHREAD_MUTEX_INITIALIZER;

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

int main(int argc, char** argv){
	if(argc!=2)
	{
		printf("Usage: %s <no_places> \n", argv[0]);
		exit(1);
	}

	//Getting the time of init
	time_init=get_current_time();

	//Semaphore intializer
	sem_init(&out_sem, SHARED, 1);
	sem_init(&places_sem, SHARED, atoi(argv[1]));

	fileHandler();
	gender = '*';

	pthread_t tid1;
	pthread_create(&tid1, NULL, mainThread, NULL);

	// Wait for mainThread to terminate
	pthread_join(tid1, NULL);

	// Print Statistics
	//TODO

	//Closing files and deleting created FIFO
	close(in_fifo);
	close(out_fifo);
	unlink(REJECTED_FIFO_PATH);

	exit(0);
}

void * request_handler(void *arg){
	// Prevent sauna overcrowding

	sem_wait(&places_sem);
	Request * req = (Request *) arg;

	served[((size_t) request_get_gender(req)) % 2]++;
	
	usleep(request_get_duration(req) * MILI_TO_MICRO);

	print_register(req, MSG_SERVED);

	int i = 0; // TODO
	sem_getvalue(&places_sem, &i); // WILL NEVER REACH THIS WITH VALUE 0
	// there's no reason for doing this inside critical section
	
	//Protecting 'gender' from being evaluated and altered simultaneally
	pthread_mutex_lock(&gender_mut);
	if(i == 0) // i == 0 is no seats empty
		gender = '*';
	pthread_mutex_unlock(&gender_mut);

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

	char filename[MAX_FILENAME_LEN];
	snprintf(filename, MAX_FILENAME_LEN, "/tmp/bal.%d", (int) getpid());

	if((out_fd=open(filename, O_RDWR|O_CREAT, FILE_PERMISSIONS)) == -1) {
		perror("Can't open LOGS FILE");
		exit(6);
	}
}

void * mainThread(void * arg){
	const ssize_t SIZEOF_REQUEST = request_get_sizeof();
	
	int i = 0;
	Request * req = malloc(SIZEOF_REQUEST);
	pthread_t threads[MAX_THREADS];

	while (read(in_fifo, req, SIZEOF_REQUEST) > 0){

		print_register(req, MSG_RECEIVED);

		// Works because F == 70 && M == 77
		received[((size_t) request_get_gender(req)) % 2]++;

		// Lock global variable gender
		pthread_mutex_lock(&gender_mut);

		if (gender == '*')
			gender = request_get_gender(req);

		// TODO check req
		// pointer to ever changing variable
		// array of requests to be freed in the end ?
		Request * tmp_req = malloc(SIZEOF_REQUEST);
		memcpy(tmp_req, req, SIZEOF_REQUEST);

		if (request_get_gender(req) == gender) {
			pthread_create(&threads[i++], NULL, request_handler, (void *) tmp_req);
		} else {
			rejected[((size_t) request_get_gender(req)) % 2]++;
			print_register(req, MSG_REJECTED);
			write(out_fifo, req, SIZEOF_REQUEST);
		}
		
		pthread_mutex_unlock(&gender_mut);
	}

	int j; // Join with all created threads
	for(j = 0; j < i; j++){
		Request ** ptr = NULL;
		pthread_join(threads[j], (void **) ptr);

		free(*ptr);
	}

	free(req);

	return NULL;
}

void print_register(Request* req, const char * msg){
	pthread_mutex_lock( &logs_mut );

	unsigned long long time_elapsed = get_current_time() - time_init;

	dprintf(out_fd, "%4llu - %4d - %4d - %4d: %c - %4d - %s\n",
		time_elapsed,				/* current time instance in miliseconds */
		pid,						/* process pid */
		(int) pthread_self(),		/* thread tid */
		request_get_serial_no(req),	/* request's serial number */
		request_get_gender(req),	/* request's gender */
		request_get_duration(req),	/* request's duration */
		msg);						/* message identifier */

	pthread_mutex_unlock( &logs_mut );
}
