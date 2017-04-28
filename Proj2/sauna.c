struct request_t {
	static int serial_no;
	char gender;
	int duration;
	int no_rejections;
}

int no_places = 0;
char gender;

char time_unit;

/**
* Simulates the steam room utilisation
*/
void * utilisation_sim(void *arg){
	int time = * (int *) arg;

	sleep(time);

	return NULL;
}

int main(int argc, char argv[][]){
	if(argc!=3)
	{
		printf("Usage: %s <no_places> <time_unit>\n", argv[0]);
		exit(1);
	}

	/* BEGIN Storing the main args */

	no_places=atoi(argv[1]);

	if(strlen(argv[2]==1))
		time_unit=argv[2][1];
	else
		exit(1);

	/* END Storing the main args */

	/* BEGIN Making and opening FIFOs */

	if (mkfifo("/tmp/rejeitados",0660)<0)
	{
		if (errno==EEXIST)
			printf("FIFO /tmp/entrada already exists\n");
		else
			printf("Can't create FIFO\n");

		exit(3);
	}

	if ((out_fd=open("/tmp/rejeitados",O_WRONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/rejeitados\n");
			exit(4);
		}

	if ((in_fd=open("/tmp/entrada",O_RDONLY)) ==-1)
		{
			printf("Can't open FIFO /tmp/entrada\n");
			exit(5);
		}

	/* END Making and oppening FIFOs */
}