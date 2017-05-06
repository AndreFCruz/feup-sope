#include "pedido.h"
#include "Request.h"
#include "io.h"

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

#define NANO_TO_MILISECONDS 0.000001
#define SECONDS_TO_MILISECONDS 1000
#define MILI_TO_MICRO		1000

#define MALE 0
#define FEMALE 1

int no_places = 0;
char gender;
int out_fifo, in_fifo;
int out_fd;
int pid;
double time_init;
int received[2];
int served[2];
int rejected[2];

sem_t out_sem;
sem_t places_sem;

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;


void print_register(Request* req, char* tip);

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

	//Storing the main args
	no_places=atoi(argv[1]);

	get_clock(&time_init);

	//Semaphore intializer
	sem_init(&out_sem, SHARED, 1);
	sem_init(&places_sem, SHARED, 1);

	pthread_mutex_init(&mut);

	fileHandler();

	pthread_t tid1;
	pthread_create(&tid1, NULL, mainThread, NULL);

	//Closing files and deleting created FIFO
	close(in_fifo);
	close(out_fifo);
	unlink(REJECTED_FIFO_PATH);

	exit(0);
}

void * utilization_sim(void *arg){
	sem_wait(&places_sem);
	Request req = * (Request *) arg;
	char* tip = "SERVED";
	
	usleep(request_get_duration(&req) * MILI_TO_MICRO);

	print_register(&req,tip);

	int i=0;
	sem_getvalue(&places_sem, &i);
	if(i==0)
		gender='';

	sem_post(&places_sem);

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

void get_clock(double *time){
	struct timespec ts_init;
	timespec_get(&ts_init, TIME_UTC);
	*time=ts_init.tv_nsec * NANO_TO_MILISECONDS;
}

void * mainThread(void * arg){
	Request req;
	pthread_t threads[MAX_THREADS];
	int i = 0;
	while (read(in_fifo, &req, sizeof(Request))>0){
		char* tip="RECEIVED";

		print_register(&req,tip);
		pthread_mutex_lock(&mut);
		
		if (request_get_gender(&req)==gender) {
			pthread_create(&threads[i], NULL, utilization_sim, (void *) &req);
			
			i++;
		}
		else {
			if (gender == '') {
				gender=request_get_gender(&req);
				pthread_create(&threads[i], NULL, utilization_sim, (void *) &req);

				i++;
			}
			else {
				char* tip="REJECTED";
				print_register(&req,tip);
			}
		}

		pthread_mutex_unlock(&mut);
	}

	int j;
	for(j = 0; j < i; j++){
		pthread_join(threads[j], NULL);
	}

	return NULL;
}

void print_register(Request* req, char* tip){
	pthread_t tid = pthread_self();
	double time_req;
	get_clock(&time_req);

	sem_wait(&out_sem);
	dprintf(out_fd, "%-5lf, %-5d, %-5lu, %-5d, %-2c, %-5d, %-10s\n", time_req-time_init, pid, tid, request_get_serial_no(req), request_get_gender(req), request_get_duration(req),  tip);
	sem_post(&out_sem);
}
