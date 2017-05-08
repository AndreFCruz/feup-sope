#ifndef __GENERATOR_T_H
#define __GENERATOR_T_H

#include <pthread.h>
#include "Request.h"

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

	// Mutex for LOGs File
	pthread_mutex_t mut_logs;

	unsigned long long START_TIME; // in miliseconds
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
void generator_print_statistics(generator_t * gen);

#endif /* __GENERATOR_T_H */
