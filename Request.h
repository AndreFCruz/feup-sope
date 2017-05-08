#ifndef __REQUEST_H
#define __REQUEST_H

#include <stdlib.h>

struct request_t;
typedef struct request_t Request;

int get_num_requests();

void set_num_requests();

int set_max_duration(int new_max);

int get_max_duration();

// Methods for Request Class
Request * new_request();

void delete_request(Request * self);

int request_get_serial_no(Request * self);

int request_get_num_rejections(Request * self);

int request_increment_rejections(Request * self);

int request_get_duration(Request * self);

int request_is_male(Request * self);

int request_is_female(Request * self);

char request_get_gender(Request * self);

// Static method
size_t request_get_sizeof();

#endif /* __REQUEST_H */
