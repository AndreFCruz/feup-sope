#define RECIEVED	0
#define REJECTED	1
#define SERVED		2

struct request_t {
	static int serial_no;
	char gender;
	int duration;
	int no_rejections;
}

int request_t::serial_no = 0;