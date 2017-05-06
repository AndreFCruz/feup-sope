#ifndef __UTILS_H
#define __UTILS_H

#define FILE_PERMISSIONS	0600
#define MAX_FILENAME_LEN	32

#define MSG_REQUEST		"PEDIDO"
#define MSG_REJECTED	"REJEITADO"
#define MSG_DISCARDED	"DESCARTADO"

const char * REQUESTS_FIFO_PATH;
const char * REJECTED_FIFO_PATH;
const char * LOGS_FILE_PATH;

#endif /* __UTILS_H */
