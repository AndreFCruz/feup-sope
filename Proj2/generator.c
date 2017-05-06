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

#define FILE_PERMISSIONS	0600

#define MAX_REJECTIONS		3
#define MAX_FILENAME_LEN	32

#define MIN_REQUESTS_GAP	5
#define MILI_TO_MICRO		1000

#define MSG_REQUEST		"PEDIDO"
#define MSG_REJECTED	"REJEITADO"
#define MSG_DISCARDED	"DESCARTADO"

// NOTE: Time units are in miliseconds

// stores command line arguments and current state of generator
struct generator_t {
	int MAX_REQUESTS;

	// File Descriptors
	int REQUESTS_FIFO;
	int REJECTED_FIFO;
	int LOGS_FILE;

	// Statistics
	int male_requests;
	int female_requests;
	int male_rejections;
	int female_rejections;
	int male_discards;
	int female_discards;
};

typedef struct generator_t generator_t;

// Methods for generator_t
generator_t * new_generator_t(char * argv[]);
void generator_open_fifos(generator_t * gen);
void generator_create_logs_file(generator_t * gen);
void generator_close_filedes(generator_t * gen);
void delete_generator_t();

void generator_log_request(generator_t * gen, Request * req);
void generator_log_reject(generator_t * gen, Request * req);
void generator_log_discard(generator_t * gen, Request * req);

void log_request_stats(int filedes, Request * req, const char * msg);

// Threads' Functions
void * requests_generator(void * arg);
void * rejected_listener(void * arg);


int main(int argc, char * argv[]) {
	if (argc != 3) {
		printf("Usage: %s <num. requests> <max. use time>\n", argv[0]);
		exit(0);
	}

	// Seed random function
	srand(time(NULL));

	// Create FIFOs if not already created
	if (mkfifo(REQUESTS_FIFO_PATH, FILE_PERMISSIONS) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}
	if (mkfifo(REJECTED_FIFO_PATH, FILE_PERMISSIONS) < 0) {
		if (errno != EEXIST) {
			perror("Error creating FIFO");
			exit(1);
		}
	}

	// Create generator_t struct and fetch command line arguments
	generator_t * generator = new_generator_t(argv);

	// Open FIFOs
	generator_open_fifos(generator);

	// Create LOGs file
	generator_create_logs_file(generator);

	// Initiate threads -- IN ORDER!!
	// listen for rejected && generate requests
	pthread_t threads[2];
	if ( pthread_create(&threads[0], NULL, rejected_listener, generator) != 0 ) {
		printf("Error creating thread rejected_listener");
		exit(1);
	}
	if ( pthread_create(&threads[1], NULL, requests_generator, generator) != 0 ) {
		printf("Error creating thread rejected_listener");
		exit(1);
	}

	// Join with threads
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	// Close opened FIFOs and files
	generator_close_filedes(generator);

	// Delete dinamically allocated memory
	delete_generator_t(generator);

	exit(0);
}

generator_t * new_generator_t(char * argv[]) {
	generator_t * gen = malloc(sizeof(generator_t));

	// Initialize variables
	gen->male_requests = 0;
	gen->female_requests = 0;
	gen->male_rejections = 0;
	gen->female_rejections = 0;
	gen->male_discards = 0;
	gen->female_discards = 0;

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
	gen->MAX_REQUESTS = tmp;

	return gen;
}

void generator_open_fifos(generator_t * gen) {
	if ( (gen->REQUESTS_FIFO = open(REQUESTS_FIFO_PATH, O_WRONLY)) == -1 ) {
		perror("Failed to open REQUESTS_FIFO");
		exit(1);
	}
	if ( (gen->REJECTED_FIFO = open(REJECTED_FIFO_PATH, O_RDONLY)) == -1 ) {
		perror("Failed to open REQUESTS_FIFO");
		exit(1);
	}
}

void generator_create_logs_file(generator_t * gen) {
	char filename[MAX_FILENAME_LEN];
	if (snprintf(filename, MAX_FILENAME_LEN, "%s%d", LOGS_FILE_PATH, getpid()) >= MAX_FILENAME_LEN) {
		printf("Error in snprintf, file name too long\n");
		exit(1);
	}
	if ( (gen->LOGS_FILE = open(filename, O_RDWR | O_CREAT, FILE_PERMISSIONS)) == -1 ) {
		perror("Failed to open/create logs file");
		exit(1);
	}
}

void generator_close_filedes(generator_t * gen) {
	close(gen->REQUESTS_FIFO);
	close(gen->REJECTED_FIFO);
	close(gen->LOGS_FILE);
}

void delete_generator_t(generator_t * gen) {
	free(gen);
}

void generator_log_request(generator_t * gen, Request * req) {
	if (request_is_male(req))
		(gen->male_requests)++;
	else
		(gen->female_requests)++;

	log_request_stats(gen->LOGS_FILE, req, MSG_REQUEST);
}

void generator_log_reject(generator_t * gen, Request * req) {
	if (request_is_male(req))
		(gen->male_rejections)++;
	else
		(gen->female_rejections)++;

	log_request_stats(gen->LOGS_FILE, req, MSG_REJECTED);
}

void generator_log_discard(generator_t * gen, Request * req) {
	if (request_is_male(req))
		(gen->male_discards)++;
	else
		(gen->female_discards)++;

	log_request_stats(gen->LOGS_FILE, req, MSG_DISCARDED);
}

void wait_for_next_request() {
	int mlsecs = MIN_REQUESTS_GAP + rand() % get_max_duration();

	usleep(mlsecs * MILI_TO_MICRO);
}

// TODO call this function
// TODO synchronize logs_file access ?
void log_request_stats(int filedes, Request * req, const char * msg) {

	dprintf(filedes, "%4d - %4d - %4d: %c - %4d - %s\n",
		000,						/* current time */ // TODO CHANGE PLACEHOLDER
		getpid(),					/* process pid */
		request_get_serial_no(req),	/* request's serial number */
		request_get_gender(req),	/* request's gender */
		request_get_duration(req),	/* request's duration */
		msg);						/* message identifier */

}

/**
 * Thread to randomly generate requests and write them to the REQUESTS_FIFO
 * @arg generator_t * State of the generator
 * @return NULL
 */
void * requests_generator(void * arg)
{
	generator_t * gen = (generator_t *) arg;

	int i;
	for (i = 0; get_num_requests() < gen->MAX_REQUESTS; ++i) {

		Request * new_req = new_request();
		generator_log_request(gen, new_req);

		// Write to fifo
		write(gen->REQUESTS_FIFO, new_req, request_get_sizeof());

		wait_for_next_request();

		// Check for congruency
		if (i != get_num_requests()) {
			printf("ERROR in requests_generator loop!\n");
			printf("i: %d ; num_requests: %d", i, get_num_requests());
			exit(1);
		}
	}

	pthread_exit(0);
}

/**
 * Listens for rejected requests from the rejected_requests FIFO
 * will exit when fifo is closed for writing (by sauna process)
 * @arg generator_t * State of the generator
 * @return NULL
 */ 
void * rejected_listener(void * arg)
{
	// Read from REJECTED_FIFO -- HANG while nothing to be read
	ssize_t SIZEOF_REQUEST = request_get_sizeof();
	generator_t * gen = (generator_t *) arg;

	Request * tmp_request = malloc(SIZEOF_REQUEST);
	while ( SIZEOF_REQUEST == read(gen->REJECTED_FIFO, tmp_request, SIZEOF_REQUEST) ) {
		request_increment_rejections(tmp_request);

		generator_log_reject(gen, tmp_request);

		if (request_get_num_rejections(tmp_request) < MAX_REJECTIONS)
			write(gen->REQUESTS_FIFO, tmp_request, SIZEOF_REQUEST);
		else
			generator_log_discard(gen, tmp_request);
	}

	delete_request(tmp_request);

	pthread_exit(0);
}

