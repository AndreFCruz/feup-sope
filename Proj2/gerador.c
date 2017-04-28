struct request_t {
	static int serial_no;
	char gender;
	int duration;
	int no_rejections;
}

int request_t::serial_no = 0;

int max_requests = 0;

int max_duration = 0;

int in_fd = 0, out_fd = 0;


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

	write(out_fd, req, sizeof(req));

	request_t::serial_no++;

	return NULL;
}*/

/* YET TO FINISH
void * listener(void *arg)
{
	struct request_t req;

	read(in_fd, req, sizeof(req));
	req.no_rejections++;
	if(req.rejections<3)
		write(out_fd, req, sizeof(req));
	
	return NULL;
}*/

int main(int argc, char argv[][]){
	srand(time(NULL));

	/* BEGIN Storing the main args */

	max_requests=atoi(argv[1]);
	max_duration=atoi(argv[2]);

	if(strlen(argv[3]==1))
		time_unit=argv[3][1];
	else
		exit(1);

	/* END Storing the main args */

	/* BEGIN Making and opening FIFOs */

	if (mkfifo("/tmp/entrada",0660)<0)
	{
		if (errno==EEXIST)
			printf("FIFO /tmp/entrada already exists\n");
		else
			printf("Can't create FIFO\n");

		exit(2);
	}

	if ((in_fd=open("/tmp/entrada",O_WRONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/entrada\n");
			exit(3);
		}

	if ((out_fd=open("/tmp/rejeitados",O_RDONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/rejeitados\n");
			exit(4);
		}

	/* END Making and oppening FIFOs */


	exit(0);
}