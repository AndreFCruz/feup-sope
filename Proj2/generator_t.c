
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include "generator_t.h"
#include "utils.h"
#include "Request.h"


void log_request_stats(generator_t * gen, Request * req, const char * msg);

generator_t * new_generator_t(char * argv[]) {
	generator_t * gen = malloc(sizeof(generator_t));

	// Initialize variables
	gen->male_requests = 0;
	gen->female_requests = 0;
	gen->male_rejections = 0;
	gen->female_rejections = 0;
	gen->male_discards = 0;
	gen->female_discards = 0;

	// Set start time
	gen->START_TIME = get_current_time();

	// Mutex to synchronize access to LOGS_FILE
	pthread_mutex_init(&(gen->mut_logs), NULL);

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

	log_request_stats(gen, req, MSG_REQUEST);
}

void generator_log_reject(generator_t * gen, Request * req) {
	if (request_is_male(req))
		(gen->male_rejections)++;
	else
		(gen->female_rejections)++;

	log_request_stats(gen, req, MSG_REJECTED);
}

void generator_log_discard(generator_t * gen, Request * req) {
	if (request_is_male(req))
		(gen->male_discards)++;
	else
		(gen->female_discards)++;

	log_request_stats(gen, req, MSG_DISCARDED);
}

void log_request_stats(generator_t * gen, Request * req, const char * msg) {
	
	pthread_mutex_lock( &(gen->mut_logs) );

	unsigned long long time_elapsed = get_current_time() - gen->START_TIME;

	dprintf(gen->LOGS_FILE, "%4llu - %4d - %4d: %c - %4d - %s\n",
		time_elapsed,		/* current time instance in miliseconds */
		getpid(),					/* process pid */
		request_get_serial_no(req),	/* request's serial number */
		request_get_gender(req),	/* request's gender */
		request_get_duration(req),	/* request's duration */
		msg);						/* message identifier */

	pthread_mutex_unlock( &(gen->mut_logs) );
}
