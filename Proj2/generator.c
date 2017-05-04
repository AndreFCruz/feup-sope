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
	int max_duration;

	// File Descriptors
	int requests_fifo;
	int rejected_fifo;
	int logs_file;
};

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <num. requests> <max. use time>\n", argv[0]);
		exit(0);
	}

	// Seed random function
	srand(time(NULL));

	// Install signal handlers (?)

	// Create generator_t struct to pass around
	struct generator_t generator;

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
	// generate requests && listen rejected


	exit(0);
}