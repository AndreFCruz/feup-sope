struct request_t {
	static int serial_no;
	char gender;
	int duration;
	int no_rejections;
}

int request_t::serial_no = 0;

int max_requests = 0;

int max_duration = 0;

int in_fifo = 0, out_fifo = 0;

char time_unit;


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

int main(int argc, char argv[][]){
	if(argc!=4)
	{
		printf("Usage: %s <no_requests> <max_duration> <time_unit>\n", argv[0]);
		exit(1);
	}

	srand(time(NULL));

	/* BEGIN Storing the main args */

	max_requests=atoi(argv[1]);
	max_duration=atoi(argv[2]);

	if(strlen(argv[3]==1))
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

	if((out_fd=open(filename, O_RDWR|O_CREAT, 0666)) == -1)	

	/* END Making and oppening FIFOs and regist_file */

	/* TODO: Send and recieve time unit */	


	exit(0);
}