#ifndef __UTILS_H
#define __UTILS_H

#define MILI_TO_MICRO		1000
#define MILI_TO_NANO		1000000
#define SECONDS_TO_MILISECONDS	1000

#define FILE_PERMISSIONS	0600
#define MAX_FILENAME_LEN	32

#define MSG_REQUEST		"PEDIDO"
#define MSG_REJECTED	"REJEITADO"
#define MSG_DISCARDED	"DESCARTADO"
#define MSG_RECEIVED	"RECEIVED"
#define MSG_SERVED		"SERVED"

extern const char * REQUESTS_FIFO_PATH;
extern const char * REJECTED_FIFO_PATH;
extern const char * LOGS_FILE_PATH;

extern const unsigned char SIGNAL_CHAR;

// Returns the current time in miliseconds (since 1970, 1st January)
unsigned long long get_current_time();

#endif /* __UTILS_H */
