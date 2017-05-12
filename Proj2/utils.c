#include "utils.h"
#include <sys/time.h>

#include <stdio.h>

const char * REQUESTS_FIFO_PATH = "/tmp/entrada";
const char * REJECTED_FIFO_PATH = "/tmp/rejeitados";
const char * LOGS_FILE_PATH = "/tmp/ger.";

const unsigned char SIGNAL_CHAR = 0x0000;

/**
 * Returns the current time in microseconds
 * in reference to 1st Jan 1970
 */
unsigned long long get_current_time() {
	struct timeval t;
	gettimeofday(&t, NULL);

	return t.tv_usec + t.tv_sec * SECONDS_TO_MILISECONDS * MILI_TO_MICRO;
}
