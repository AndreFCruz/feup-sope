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

#define DIR_IDX		1
#define PRED_IDX	2
#define ARG_IDX		3
#define FUNC_IDX 	4


struct args_t {
	char * home_dir;
	char ** argv;

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
#if TEST
		printf("Entry: %s\n", entry->d_name);
#endif

		bool useful;
		if ( (useful = (args.pred)(args.arg_ptr, entry)) ) {
			(args.func)(entry);
		}

#if TEST
		printf("Predicate eval: %s\n", useful ? "True" : "False");
#endif

		if (entry->d_type == DT_DIR) {
			// open dir
			// fork child with same function
			
			DIR * sub_dir = opendir(entry->d_name);
			// may need to call telldir for absolute path (?)

			pid_t child;
			if ( (child = fork()) == 0 ) { // child
				execl(args.home_dir, *(args.argv));
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

#if TEST
	printf("testing name %s\n", name);
#endif

	return strncmp(file->d_name, name, strlen(name)) == 0 ? true : false;
}

bool typeEquals(void * arg_ptr, struct dirent * file) {
	unsigned char type = * ((unsigned char *) arg_ptr);

#if TEST
	printf("testing type %c\n", type);
#endif

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

	struct args_t args;
	args.argv = argv;
	if ( getwd(args.home_dir) == NULL ) {
		perror("getwd failed in main");
		exit(1);
	}

#if TEST
	printf("Main pre-loading starting\n");
#endif

	// Set Predicate
	if (strncmp(argv[PRED_IDX], "-name", 5) == 0) {
		args.pred = nameEquals;
		args.arg_ptr = &argv[ARG_IDX];
	} else if (strncmp(argv[PRED_IDX], "-type", 5) == 0) {
		args.pred = typeEquals;
		// set arg to file type
	} else if (strncmp(argv[PRED_IDX], "-perm", 5) == 0) {
		args.pred = permEquals;
	}

	// Set Directory
	if ( (args.dir = opendir(argv[DIR_IDX])) == NULL ) {
		perror("opendir failed");
		exit(0);
	}

	// Set Function
	if ( strncmp(argv[FUNC_IDX], "-print", 6) == 0 ) {
		args.func = printEntry;
	} else if ( strncmp(argv[FUNC_IDX], "-delete", 7) == 0 ) {
		args.func = deleteEntry;
	} else if ( strncmp(argv[FUNC_IDX], "-exec", 5) == 0 ) { // TODO handle other arguments
		;
	}

#if TEST
	printf("Main pre-loading finished\n");
#endif

	handleDirectory(args);
	closedir(args.dir);

	exit(0);
}

