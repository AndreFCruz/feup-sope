#include <time.h>
#include <stdlib.h>
#include "Request.h"

typedef enum {
	MALE,
	FEMALE
} gender_t;

struct request_t {
	int serial_no;
	gender_t gender;
	int duration;
	int num_rejections;
};

static int num_requests = 0;
static int max_duration = 0;

int get_num_requests() {
	return num_requests;
}

void set_num_requests(){
	num_requests++;
}

int get_max_duration() {
	return max_duration;
}

int set_max_duration(int new_max) {
	if (max_duration != 0)
		return 0; // error if max_duration already set
	return max_duration = new_max;
}

// TODO srand should be called in main -- seed for rand function

// Randomly generate a new Sauna request. Increment global counter
Request * new_request() {
	Request * self = (Request *) malloc(sizeof(Request));

	self->serial_no = num_requests;
	self->gender = rand() % 2 ? MALE : FEMALE;
	self->duration = rand() % max_duration + 1;
	self->num_rejections = 0;

	return self;
}

void delete_request(Request * self) {
	free(self);
}

int request_get_serial_no(Request * self) {
	return self->serial_no;
}

int request_get_num_rejections(Request * self) {
	return self->num_rejections;
}

int request_increment_rejections(Request * self) {
	return ++(self->num_rejections);
}

int request_get_duration(Request * self) {
	return self->duration;
}

int request_is_male(Request * self) {
	return (self->gender == MALE);
}

int request_is_female(Request * self) {
	return (self->gender == FEMALE);
}

char request_get_gender(Request * self) {
	return self->gender == MALE ? 'M' : 'F';
}

size_t request_get_sizeof() {
	return sizeof(Request);
}



