#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
// #include <sys/stat.h>

#define TEST 0

#define MAX_DIR_NAME_LEN	256

typedef int bool;
enum { false,  true };

#define DIR_IDX		1
#define PRED_IDX	2
#define ARG_IDX		3
#define FUNC_IDX 	4

char * const * get_argv(int argc, char * const argv[], const char * sub_dir);

struct args_t {
	char * home_dir;
	char ** argv;
	int argc;

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
			printf("Entry name: %s\n", entry->d_name);
			// may need to call telldir for absolute path (?)

			pid_t child;
			if ( (child = fork()) == 0 ) { // child
				char * const * new_argv = get_argv(args.argc, args.argv, entry->d_name);
				
				printf("Creating child at: %s\n", new_argv[DIR_IDX]);

				// if (execv(args.home_dir, new_argv) == -1) {
				// 	perror("exec failed");
				// 	return false;
				// }

			} else if (child == -1) {
				perror("error in fork");
				return false;
			} else {
				printf("Creating child %d\n", child);
			}
		}
	}

	int status;
	// Wait for child results -- Handle Exit Status ?
	while( (waitpid(-1, &status, 0)) != -1 );

	return true; // FUTURE: change
}

char * const * get_argv(int argc, char * const argv[], const char * sub_dir) {

    // allocate memory and copy strings
    char** new_argv = malloc((argc+1) * sizeof(*new_argv));
    for(int i = 0; i < argc; ++i)
    {
        size_t length = strlen(argv[i])+1;
        new_argv[i] = malloc(length);
        memcpy(new_argv[i], argv[i], length);
    }
    new_argv[argc] = NULL;

    int arg1_len = strlen(new_argv[DIR_IDX]) + strlen(sub_dir) + 1;
    char * arg1 = malloc(arg1_len);
    memset(arg1, 0, arg1_len);
    strcat(arg1, new_argv[DIR_IDX]);
    strncat(arg1, "/", 1);
    strcat(arg1, sub_dir);

    new_argv[DIR_IDX] = arg1;

    // FUTURE ?
    // // free memory
    // for(int i = 0; i < argc; ++i)
    // {
    //     free(new_argv[i]);
    // }
    // free(new_argv);

    return new_argv;
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
	char * wd = malloc(MAX_DIR_NAME_LEN);
	if (getcwd(wd, MAX_DIR_NAME_LEN) == NULL) {
		perror("printEntry failed getwd");
	}

	printf("%s/%s\n", wd, entry->d_name);

	return true;
}

bool deleteEntry(struct dirent * entry) {
	char * wd = malloc(MAX_DIR_NAME_LEN);
	if (getcwd(wd, MAX_DIR_NAME_LEN) == NULL) {
		perror("deleteEntry failed getwd");
	}

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
	args.argv = argv;	// save argv
	args.argc = argc;	// save argc
	args.home_dir = malloc(MAX_DIR_NAME_LEN);
	if ( getcwd(args.home_dir, MAX_DIR_NAME_LEN) == NULL ) {
		perror("getwd failed in main");
		printf("yoyogetwd failed\n");
		exit(1);
	}
	strncat(args.home_dir, "/", MAX_DIR_NAME_LEN);
	strncat(args.home_dir, argv[0], MAX_DIR_NAME_LEN);

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

	printf("Opening dir: %s\n", argv[DIR_IDX]);
	printf("Home dir: %s\n", args.home_dir);

	// unsigned str_len = strlen(args.home_dir) + strlen(argv[0]) + 1;
	// char * str = malloc(str_len);
	// memset(str, 0, sizeof(str_len));
	// strcat(str, args.home_dir);

	handleDirectory(args);

	closedir(args.dir);

	exit(0);
}

