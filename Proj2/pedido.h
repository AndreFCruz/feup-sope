#define RECIEVED	0
#define REJECTED	1
#define SERVED		2

struct {
	int serial_no;
	char gender;
	int duration;
	int no_rejections;
} request_t;

//TODO: How can serial_no be a static counter?