#include "pedido.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define SHARED 0

int no_places = 0;
char gender;
int out_fifo, in_fifo;
int out_fd;
int time_init;	/* Initial time */

sem_t out_sem;
sem_t places_sem;

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;

/**
* Simulates the steam room utilisation
*/
void * utilisation_sim(void *arg);

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

	//Semaphore intializer
	seminit(&out_sem, SHARED, 1);

	fileHandler();

	//Closing files and deleting created FIFO
	close(in_fifo);
	close(out_fifo);
	unlink("/tmp/rejeitados");

	exit(0);
}

void * utilisation_sim(void *arg){
	struct request_t req = * (struct request_t *) arg;
	pthread_t tid = pthread_self();
	int time_instant=0;
	char* tip = 'SERVED';

	sleep(req.duration);
	sem_wait(&out_sem);
	dprintf(out_fd, "%-5d, %-5d, %-5d, %-5d, %-2c, %-5d, %-10s\n", time_instant, pid, tid, req.serial_no, req.gender, req.duration, /*(tip ?)*/);
	sem_post(&out_sem);

	pthread_mutex_lock(&places_sem); 
	no_places--;
 	pthread_mutex_unlock(&places_sem); 

	return NULL;
}

void fileHandler(){
	if (mkfifo("/tmp/rejeitados",0660)<0)
	{
		if (errno==EEXIST)
			printf("FIFO /tmp/rejeitados already exists\n");
		else
		{
			printf("Can't create FIFO\n");
			exit(3);
		}
	}

	if ((in_fifo=open("/tmp/entrada",O_RDONLY)) ==-1)
	{
		printf("Can't open FIFO /tmp/entrada\n");
		exit(5);
	}

	if ((out_fifo=open("/tmp/rejeitados",O_WRONLY)) ==-1)
	{
		printf("Can't open FIFO /tmp/rejeitados\n");
		exit(4);
	}

	int pid = getpid();
	char * filename=malloc(sizeof(char)*50);
	snprintf(filename, 50, "/tmp/bal.%d", pid);

	if((out_fd=open(filename, O_RDWR|O_CREAT, 0666)) == -1)
	{
		printf("Can't open FIFO %s\n",filename);
		exit(6);
	}
}