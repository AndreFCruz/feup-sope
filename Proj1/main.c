#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

// SFIND
// simplified version of find

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
	char home_dir[MAX_DIR_NAME_LEN];
	char ** argv;
	int argc;

	DIR * dir;

	bool (*pred)(const struct args_t * args, const struct dirent * file);	// predicate that receives a file
	bool (*func)(const struct args_t * args, const struct dirent * file);	// function to be executed on file
};

/**
 * Signal Handler for SigInt (CTRL + C)
 */
void sigIntHandler(int signo) {
	(void) signo; // Silence Unused Parameter Warning

	// char ans;

	// printf("Are you sure you want to terminate (Y/N)?\n");
	// scanf("%c", &ans);

	// if( ans == 'y' || ans == 'Y' ) {
 //  		exit(0);
	// }
	exit(0);
}


bool handleDirectory(const struct args_t * args) {

	struct dirent * entry;

	// Recursively create processes to handle sub directories
	while ( (entry = readdir(args->dir)) != NULL ) {
#if TEST == 2
		printf("Entry: %s\n", entry->d_name);
#endif
		// Ignore entries "." and ".."
		if ( strncmp(entry->d_name, ".", 1) == 0 || strncmp(entry->d_name, "..", 2) == 0 )
			continue;

		bool useful;
		if ( (useful = (args->pred)(args, entry)) ) {
			if ( ! (args->func)(args, entry) )
				printf("Function \"%s\" failed. Entry was \"%s\"", args->argv[FUNC_IDX], entry->d_name);
		}

		if ( entry->d_type == DT_DIR ) {
			// open dir
			// fork child with same function
	
			DIR * sub_dir = opendir(entry->d_name);

			pid_t child;
			if ( (child = fork()) == 0 ) { // child
				char * const * new_argv = get_argv(args->argc, args->argv, entry->d_name);

#if TEST
				printf("Child at: %s\n", new_argv[DIR_IDX]);
#endif

				if (execv(args->home_dir, new_argv) == -1) {
					perror("exec failed");
					return false;
				}

			} else if (child == -1) {
				perror("error in fork");
				return false;
			} else {
#if TEST
				printf("Created child %d\n", child);
#endif
			}
		}
	}

	int status;
	// Wait for child results -- Handle Exit Status ?
	while( (waitpid(-1, &status, 0)) != -1 );

	return true; // FUTURE: change
}

char * getAbsPath(const char * base_dir, const char * entry_name) {
	int abs_len = strlen(base_dir) + strlen(entry_name) + 1;

	char * path = malloc(abs_len);
	memset(path, 0, abs_len);
	strcat(path, base_dir);
	strncat(path, "/", 1);
	strcat(path, entry_name);

	return path;
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

	new_argv[DIR_IDX] = getAbsPath(argv[DIR_IDX], sub_dir);

	// FUTURE ? 
	// TODO perguntar se se deve libertar a memória do argv do child process (child copia?)

	// // free memory
	// for(int i = 0; i < argc; ++i)
	// {
	//     free(new_argv[i]);
	// }
	// free(new_argv);

	return new_argv;
}

/** Predicates **/
bool nameEquals(const struct args_t * args, const struct dirent * file) {
	char * name = args->argv[ARG_IDX];

#if TEST == 3
	printf("testing name %s\n", name);
#endif

	return strncmp(file->d_name, name, strlen(name)) == 0 ? true : false;
}

int getTypeInt(char c) {
	switch (c) {
	case 'f':
		return DT_REG;
	case 'd':
		return DT_DIR;
	case 'l':
		return DT_LNK;
	default:
		return -1;
	}
}

bool typeEquals(const struct args_t * args, const struct dirent * file) {
	char type = *(args->argv[ARG_IDX]);

#if TEST == 3
	printf("testing type %c\n", type);
#endif

	return getTypeInt(type) == file->d_type;
}

bool permEquals(const struct args_t * args, const struct dirent * file) {
	unsigned perm = strtol(args->argv[ARG_IDX], NULL, 8);



	return false;
}
/** END OF Predicates **/


/** Functions **/
bool printEntry(const struct args_t * args, const struct dirent * entry) {
	printf(" -> %s/%s\n", args->argv[DIR_IDX], entry->d_name);

	return true;
}

bool deleteEntry(const struct args_t * args, const struct dirent * entry) {
	char * str = getAbsPath(args->argv[DIR_IDX], entry->d_name);
	int ret;

#if TEST
	printf("Deleted %s\n", str);
	ret = 0;
#else
	ret = remove(str);
	free(str);
#endif

	return (ret != -1);
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
	if ( getcwd(args.home_dir, MAX_DIR_NAME_LEN) == NULL ) {
		perror("getwd failed in main");
		exit(1);
	}
	strncat(args.home_dir, "/", MAX_DIR_NAME_LEN);
	strncat(args.home_dir, argv[0], MAX_DIR_NAME_LEN);

	// Set Predicate
	if (strncmp(argv[PRED_IDX], "-name", 5) == 0) {
		args.pred = nameEquals;
	} else if (strncmp(argv[PRED_IDX], "-type", 5) == 0) {
		args.pred = typeEquals;
		
		if ( getTypeInt(*argv[ARG_IDX]) == -1 ) {
			printf("File type not compatible. Please use one of \"f d l\".\n");
			exit(1);
		}

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
	printf("Opening dir: %s\n", argv[DIR_IDX]);
#endif
#if TEST == 3
	printf("Home dir:    %s\n", args.home_dir);
#endif


	handleDirectory(&args);

	closedir(args.dir);

	exit(0);
}

