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
void * utilization_sim(void *arg);

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
	gender='*';

	pthread_t tid1;
	pthread_create(&tid1, NULL, mainThread, NULL);

	//Closing files and deleting created FIFO
	close(in_fifo);
	close(out_fifo);
	unlink(REJECTED_FIFO_PATH);

	exit(0);
}

void * utilization_sim(void *arg){
	//Prevent that more than one request get served at a time
	sem_wait(&places_sem);
	ssize_t SIZEOF_REQUEST = request_get_sizeof();
	Request * req = malloc(SIZEOF_REQUEST);
	req = (Request *) arg;
	char* tip = "SERVED";

	served[(int) request_get_gender(req)%2]++;
	
	usleep(request_get_duration(req) * MILI_TO_MICRO);

	print_register(req,tip);

	int i = 0; // TODO
	sem_getvalue(&places_sem, &i); // WILL NEVER REACH THIS WITH VALUE 0
	// there's no reason for doing this inside critical section
	
	//Protecting 'gender' from being evaluated and altered simultaneally
	pthread_mutex_lock(&gender_mut);
	if(i == 0)
		gender = '*';
	pthread_mutex_unlock(&gender_mut);

	sem_post(&places_sem);
	free(req);
	return NULL;
}

void fileHandler(){
	if (mkfifo(REQUESTS_FIFO_PATH, 0666) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}
	if (mkfifo(REJECTED_FIFO_PATH, 0666) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}

	if ((in_fifo=open(REQUESTS_FIFO_PATH,O_RDONLY)) ==-1)
	{
		printf("Can't open FIFO REQUESTS_FIFO_PATH\n");
		exit(5);
	}

	if ((out_fifo=open(REJECTED_FIFO_PATH,O_WRONLY)) ==-1)
	{
		printf("Can't open FIFO FIFO_REJECTED\n");
		exit(4);
	}

	pid = getpid();
	char * filename=malloc(sizeof(char)*50);
	snprintf(filename, 50, "/tmp/bal.%d", pid);

	if((out_fd=open(filename, O_RDWR|O_CREAT, 0666)) == -1)
	{
		printf("Can't open FIFO %s\n",filename);
		exit(6);
	}
}

void * mainThread(void * arg){
	ssize_t SIZEOF_REQUEST = request_get_sizeof();
	Request * req = malloc(SIZEOF_REQUEST);
	req = (Request *) arg;
	pthread_t threads[MAX_THREADS];
	int i = 0;
	while (read(in_fifo, req, sizeof(SIZEOF_REQUEST))>0){
		char* tip="RECEIVED";

		print_register(req,tip);

		//Process one request at a time, so that 'gender' doen't get corrupted
		pthread_mutex_lock(&gender_mut);
		received[(int) request_get_gender(req)%2]++;

		if (request_get_gender(req)==gender) {
			pthread_mutex_unlock(&gender_mut);
			pthread_create(&threads[i], NULL, utilization_sim, (void *) req);
			
			i++;
		}
		else {
			if (gender == '*') {
				pthread_mutex_unlock(&gender_mut);
				gender=request_get_gender(req);
				pthread_create(&threads[i], NULL, utilization_sim, (void *) req);

				i++;
			}
			else {
				rejected[(int) request_get_gender(req)%2]++;
				pthread_mutex_unlock(&gender_mut);
				char* tip="REJECTED";
				print_register(req,tip);
			}
		}
	}

	int j;
	for(j = 0; j < i; j++){
		pthread_join(threads[j], NULL);
	}

	free(req);

	return NULL;
}

void print_register(Request* req, const char* tip){
	pthread_mutex_lock( &logs_mut );

	unsigned long long time_elapsed = get_current_time() - time_init;

	dprintf(out_fd, "%4llu - %4d - %4lu - %4d: %c - %4d - %s\n",
		time_elapsed,				/* current time instance in miliseconds */
		pid,						/* process pid */
		pthread_self(),				/* thread tid */
		request_get_serial_no(req),	/* request's serial number */
		request_get_gender(req),	/* request's gender */
		request_get_duration(req),	/* request's duration */
		tip);						/* message identifier */

	pthread_mutex_unlock( &logs_mut );
}
