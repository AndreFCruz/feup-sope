#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "Request.h"
#include "io.h"

#define MAX_REJECTIONS		3
#define MAX_FILENAME_LEN	32

// NOTE: Time units are in miliseconds

// stores command line arguments and current state of generator
struct generator_t {
	int MAX_REQUESTS;

	// File Descriptors
	int requests_fifo;
	int rejected_fifo;
	int logs_file;
};

void * requests_generator(void * arg);
void * rejected_listener(void * arg);

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <num. requests> <max. use time>\n", argv[0]);
		exit(0);
	}

	// Seed random function
	srand(time(NULL));

	// Install signal handlers (?)

	// Create generator_t struct and fetch command line arguments
	struct generator_t generator;
	long tmp;
	if ( (tmp = strtol(argv[2], NULL, 10)) == 0 ) {
		perror("Error converting third command line argument to int");
		exit(1);
	}
	set_max_duration(tmp);

	if ( (tmp = strtol(argv[1], NULL, 10)) == 0 ) {
		perror("Error converting third command line argument to int");
		exit(1);
	}
	generator.MAX_REQUESTS = tmp;


	// Create FIFOs if not already created
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

	// Open FIFOs
	if ( (generator.requests_fifo = open(REQUESTS_FIFO_PATH, O_WRONLY)) == -1 ) {
		perror("Failed to open requests_fifo");
		exit(1);
	}
	if ( (generator.rejected_fifo = open(REJECTED_FIFO_PATH, O_RDONLY)) == -1 ) {
		perror("Failed to open requests_fifo");
		exit(1);
	}

	// Create LOGs file
	char filename[MAX_FILENAME_LEN];
	if (snprintf(filename, MAX_FILENAME_LEN, "%s%d", LOGS_FILE_PATH, getpid()) >= MAX_FILENAME_LEN) {
		printf("Error in snprintf, file name too long\n");
		exit(1);
	}
	if ( (generator.logs_file = open(filename, O_RDWR | O_CREAT, 0666)) == -1 ) {
		perror("Failed to open/create logs file");
		exit(1);
	}

	// Initiate threads
	// generate requests && listen for rejected


	exit(0);
}

/**
 * Thread to randomly generate requests and write them to the requests_fifo
 * @arg struct generator_t * State of the generator
 * @return NULL
 */
void * requests_generator(void * arg)
{
	struct generator_t * generator = (struct generator_t *) arg;

	int i;
	for (i = 0; get_num_requests() < generator->MAX_REQUESTS; ++i) {

		Request * new_req = new_request();

		// Acquire lock on requests_fifo
		// TODO

		// Write to fifo
		write(generator->requests_fifo, new_req, request_get_sizeof());

		// Check for congruency
		if (i != get_num_requests()) {
			printf("ERROR in requests_generator loop!\n");
			printf("i: %d ; num_requests: %d", i, get_num_requests());
			exit(1);
		}
	}

	pthread_exit(0);
}


void * rejected_listener(void * arg)
{
	// Read from rejected_fifo and HANG while nothing to be read

	pthread_exit(0);
}

