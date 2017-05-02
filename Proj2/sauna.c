#include "pedido.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int no_places = 0;
char gender;
int out_fifo, in_fifo;
int out_fd;
char time_unit;

/*
void writeRegist(int thread_id, struct request_t req)
{
	// Write formatted (time_instant ?) , pid, tid, req.serial_no, req.gender, req.duration, (tip ?)
}
*/

/**
* Simulates the steam room utilisation
*/
void * utilisation_sim(void *arg){
	int time = * (int *) arg;

	sleep(time);

	return NULL;
}

int main(int argc, char** argv){
	if(argc!=3)
	{
		printf("Usage: %s <no_places> <time_unit>\n", argv[0]);
		exit(1);
	}

	/* BEGIN Storing the main args */

	no_places=atoi(argv[1]);

	if(strlen(argv[2])==1)
		time_unit=argv[2][1];
	else
		exit(1);

	/* END Storing the main args */

	/* BEGIN Making and opening FIFOs and regist_file */

	if (mkfifo("/tmp/rejeitados",0660)<0)
	{
		if (errno==EEXIST)
			printf("FIFO /tmp/entrada already exists\n");
		else
			printf("Can't create FIFO\n");

		exit(3);
	}

	if ((out_fifo=open("/tmp/rejeitados",O_WRONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/rejeitados\n");
			exit(4);
		}

	if ((in_fifo=open("/tmp/entrada",O_RDONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/entrada\n");
			exit(5);
		}

	int pid = getpid();
	char * filename=malloc(sizeof(char)*50);
	snprintf(filename, 50, "/tmp/bal.%d", pid);

	if((out_fd=open(filename, O_RDWR|O_CREAT, 0666)) == -1)
	{
		printf("Can't open FIFO %s\n",filename);
		exit(6);
	}

	/* END Making and oppening FIFOs and regist_file */

	exit(0);
}