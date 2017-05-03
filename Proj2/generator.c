#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// stores command line arguments and current state of generator
struct generator_t {

}

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <num. requests> <max. use time>\n", argv[0]);
		exit(0);
	}


	// Install signal handlers (?)

	// Create/Open FIFOs and other files

	// Create generator_t struct to pass around

	// Initiate threads
	// generate requests && listen rejected


	exit(0);
}