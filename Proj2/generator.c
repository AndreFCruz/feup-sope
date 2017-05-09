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
#include "generator_t.h"
#include "Request.h"
#include "utils.h"

#define MAX_REJECTIONS		3

#define MIN_REQUESTS_GAP	5
#define MILI_TO_MICRO		1000

// NOTE: Time units are in miliseconds

// Threads' Functions
void * requests_generator(void * arg);
void * rejected_listener(void * arg);

pthread_mutex_t reject_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t request_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t discard_mut = PTHREAD_MUTEX_INITIALIZER;

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

	// Print execution statistics
	generator_print_statistics(generator);

	// Close opened FIFOs and files
	generator_close_filedes(generator);

	// Delete FIFOs
	unlink(REJECTED_FIFO_PATH);
	unlink(REQUESTS_FIFO_PATH);

	// Delete dinamically allocated memory
	delete_generator_t(generator);

	exit(0);
}

void wait_for_next_request() {
	int mlsecs = MIN_REQUESTS_GAP + rand() % get_max_duration();

	usleep(mlsecs * MILI_TO_MICRO);
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

		pthread_mutex_lock( &request_mut );
		generator_log_request(gen, new_req);
		pthread_mutex_unlock( &request_mut );

		// Write to fifo
		write(gen->REQUESTS_FIFO, new_req, request_get_sizeof());

		wait_for_next_request();
		
		delete_request(new_req);

		// Check for congruency
		if (i != (get_num_requests() - 1)) {
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
	int bytes_read;

	int requests_processed = 0;

	while ( (bytes_read = read(gen->REJECTED_FIFO, tmp_request, SIZEOF_REQUEST)) > 0 ) {
		if (1 == bytes_read) {
			requests_processed++;
			continue;
		} else if (SIZEOF_REQUEST != bytes_read) {
			printf("ERROR: Listener read %d bytes.", bytes_read);
			exit(1);
		}

		request_increment_rejections(tmp_request);

		pthread_mutex_lock( &reject_mut );
		generator_log_reject(gen, tmp_request);
		pthread_mutex_unlock( &reject_mut );

		if (request_get_num_rejections(tmp_request) < MAX_REJECTIONS)
			write(gen->REQUESTS_FIFO, tmp_request, SIZEOF_REQUEST);
		else {
			pthread_mutex_lock( &discard_mut );
			generator_log_discard(gen, tmp_request);
			pthread_mutex_unlock( &discard_mut );
			requests_processed++;
		}

		// Check if all requests were processed
		if (requests_processed == gen->MAX_REQUESTS)
			break;
	}

	free(tmp_request);

	pthread_exit(0);
}

