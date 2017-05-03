#ifndef __REQUEST_H
#define __REQUEST_H

struct request_t;
typedef struct request_t Request;

Request * new_request();

void delete_request(Request * self);

int request_get_serial_no(Request * self);

int request_get_num_rejections(Request * self);

int request_increment_rejections(Request * self);

int request_get_duration(Request * self);

int request_is_male(Request * self);

int request_is_female(Request * self);

#endif /* __REQUEST_H */