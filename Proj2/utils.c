#include "utils.h"
#include <sys/time.h>

#include <stdio.h>



const char * REQUESTS_FIFO_PATH = "/tmp/entrada";
const char * REJECTED_FIFO_PATH = "/tmp/rejeitados";
const char * LOGS_FILE_PATH = "/tmp/ger.";

unsigned long long get_current_time() {
	struct timeval t;
	gettimeofday(&t, NULL);

	return (t.tv_usec / MILI_TO_MICRO) + (t.tv_sec * SECONDS_TO_MILISECONDS);
}
