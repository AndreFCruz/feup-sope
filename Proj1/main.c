#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
// #include <sys/stat.h>

#define TEST 1

typedef int bool;
enum { false,  true };


struct args_t {
	DIR * dir;

	void * arg_ptr;
	bool (*pred)(void * arg_ptr, struct dirent * file);	// predicate that receives a file
	bool (*func)(struct dirent * file);	// function to be executed on file
};

/**
 * Signal Handler for SigInt (CTRL + C)
 */
void sigIntHandler(int signo) {
	char ans;

	printf("Are you sure you want to terminate (Y/N)?\n");
	scanf("%c", &ans);

	if( ans == 'y' || ans == 'Y' ) {
  		exit(0);
	}
}


bool handleDirectory(struct args_t args) {

	struct dirent * entry;

	// Recursively create processes to handle sub directories
	while ( (entry = readdir(args.dir)) != NULL ) {

		bool useful;
		if ( (useful = (* (args.pred))(args.arg_ptr, entry)) ) {
			(* args.func)(entry);
		}

		if (entry->d_type == DT_DIR) {
			// open dir
			// fork child with same function
			
			DIR * sub_dir = opendir(entry->d_name);
			// may need to call telldir for absolute path (?)

			pid_t child;
			if ( (child = fork()) == 0 ) { // child
				struct args_t sub_args = args;
				sub_args.dir = sub_dir;
				if ( chdir(entry->d_name) == -1 )
					return false;

				return handleDirectory(sub_args);
			} else if (child == -1) {
				perror("error in fork");
				return false;
			}
		}
	}

	closedir(args.dir);

	int status;
	// Wait for child results -- Handle Exit Status ?
	while( (waitpid(-1, &status, 0)) != -1 );

	return true; // FUTURE: change
}

/** Predicates **/
bool nameEquals(void * arg_ptr, struct dirent * file) {
	char * name = * ((char **) arg_ptr);

	return strncmp(file->d_name, name, strlen(name)) == 0 ? true : false;
}

bool typeEquals(void * arg_ptr, struct dirent * file) {
	unsigned char type = * ((unsigned char *) arg_ptr);

	return type == file->d_type;
}

bool permEquals(void * arg_ptr, struct dirent * file) {
	// obtain permissions from dirent
	// has absolute path ?

	return false;
}
/** END OF Predicates **/


/** Functions **/
bool printEntry(struct dirent * entry) {
	char * wd;
	getwd(wd);
	printf("%s/%s", wd, entry->d_name);

	return true;
}

bool deleteEntry(struct dirent * entry) {
	char * wd;
	getwd(wd);

	return remove(entry->d_name) != -1;
}
/** END OF Functions **/

int main(int argc, char* argv[])
{
	if (argc < 5) {
		printf("Invalid number of arguments\n");
		// TODO print usage
		exit(0);
	}
	signal(SIGINT, sigIntHandler);

	char * dir = argv[1];
	char * pred = argv[2];
	char * func = argv[4];

	struct args_t args;

	// Set Predicate
	if (strncmp(pred, "-name", 5) == 0) {
		args.pred = &nameEquals;
	} else if (strncmp(pred, "-type", 5) == 0) {
		args.pred = &typeEquals;
	} else if (strncmp(pred, "-perm", 5) == 0) {
		args.pred = &permEquals;
	}

	// Set Directory
	if ( (args.dir = opendir(dir)) == NULL ) {
		perror("opendir failed");
		exit(0);
	}

	// Set Function
	if ( strncmp(func, "-print", 6) == 0 ) {
		args.func = &printEntry;
	} else if ( strncmp(func, "-delete", 7) == 0 ) {
		args.func = &deleteEntry;
	} else if ( strncmp(func, "-exec", 5) == 0 ) { // TODO handle other arguments
		;
	}

#if TEST
	printf("Main pre-loading\n");
#endif

	handleDirectory(args);
	closedir(args.dir);

	exit(0);
}

