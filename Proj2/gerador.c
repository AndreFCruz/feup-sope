#include "pedido.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int max_requests = 0;

int max_duration = 0;

int in_fifo = 0, out_fifo = 0;

int out_fd = 0;

char time_unit;

int pid;


/* YET TO FINISH 
void * gen_request (void *arg)
{
	struct request_t req;
		
	int gen = rand() % 2;
	if(gen==0)
		req.gender='M';
	else
		req.gender='F';

	int req.duration = rand() % max_duration;

	write(out_fifo, req, sizeof(req));

	request_t::serial_no++;

	return NULL;
}*/

/*
void writeRegist(struct request_t req)
{
	// Write formatted (time_instant ?) , pid, req.serial_no, req.gender, req.duration, (tip ?)
}
*/

/* YET TO FINISH
void * listener(void *arg)
{
	struct request_t req;

	read(in_fifo, req, sizeof(req));
	req.no_rejections++;
	if(req.rejections<3)
		write(out_fifo, req, sizeof(req));
	
	return NULL;
}*/

int main(int argc, char** argv){
	if(argc!=4)
	{
		printf("Usage: %s <no_requests> <max_duration> <time_unit>\n", argv[0]);
		exit(1);
	}

	srand(time(NULL));

	/* BEGIN Storing the main args */

	max_requests=atoi(argv[1]);
	max_duration=atoi(argv[2]);

	if(strlen(argv[3])==1)
		time_unit=argv[3][1];
	else
		exit(2);

	/* END Storing the main args */

	/* BEGIN Making and opening FIFOs and regist_file */

	if (mkfifo("/tmp/entrada",0660)<0)
	{
		if (errno==EEXIST)
			printf("FIFO /tmp/entrada already exists\n");
		else
			printf("Can't create FIFO\n");

		exit(3);
	}

	if ((in_fifo=open("/tmp/entrada",O_WRONLY)) == -1)
	{
		printf("Can't open FIFO /tmp/entrada\n");
		exit(4);
	}

	if ((out_fifo=open("/tmp/rejeitados",O_RDONLY)) == -1)
	{
		printf("Can't open FIFO /tmp/rejeitados\n");
		exit(5);
	}

	pid = getpid();
	char * filename=malloc(sizeof(char)*50);
	snprintf(filename, 50, "/tmp/ger.%d", pid);

	if((out_fd=open(filename, O_RDWR|O_CREAT, 0666)) == -1)
	{
		printf("Can't open FIFO %s\n",filename);
		exit(6);
	}

	/* END Making and oppening FIFOs and regist_file */

	/* TODO: Send and recieve time unit */	


	exit(0);
}