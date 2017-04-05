#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
// #include <sys/stat.h>

void searchName(char* dir, char* name, char* command[]);

void sigIntHandler(int signo);

void printName(char* name, char* dir);

int main(int argc, char* argv[])
{
	signal(SIGINT, sigIntHandler);

	char * dir = argv[1];
	char * func = argv[2];

	// if(strcmp(argv[2],"-name")==0)
	// {
 // 		printf("NAME=%s\n",argv[3]);
 // 		printf("CMD=%s\n",&argv[4][0]);
		
	// 	searchName(argv[1], argv[3], &argv[4]);
	// }


	exit(0);
}


void * handleDirectory(void * arg) {
	DIR * dir = (char *) arg;

	struct dirent * entry;

	// Recursively create processes to handle sub directories
	while ( (entry = readdir(dir)) != NULL ) {

	}


	// Wait for child results

}

/**
 * Signal Handler for SigInt (CTRL + C)
 */
void sigIntHandler(int signo) {
	char ans;

	printf("Are you sure you want to terminate (Y/N)?\n");
	scanf("%c", &ans);

	if( ans == 'y' || ans == 'Y' )
	{
  		exit(0);
	}
}



// void searchName(char* dir, char* name, char* command[]){

// 	if(strcmp(command[0],"-print")==0)
// 	{
// 		/*
// 		*	Mostra os ficheiros encontrados
// 		*/
// 		printName(name, dir);

// 		exit(0);
// 	}

// 	if(strcmp(command[0],"-delete")==0)
// 	{
// 		/*
// 		*	Apaga os ficheiros encontrados
// 		*/

// 		exit(0);
// 	}

// 	if(strcmp(command[0],"-exec")){
// 		/*
// 		*
// 		*/
// 	}
// }

void printName(char* name, char* dir)
{
	
 	printf("DIR=%s\n",dir);

    DIR* dirp;
    struct dirent* direntp;
 	struct stat stat_buf;

    if ((dirp = opendir(dir)) == NULL) {
        perror(dir);
        exit(1);}
    
    chdir(dir);

	while ((direntp = readdir( dirp)) != NULL)
 {
  if (lstat(direntp->d_name, &stat_buf)==-1)   // testar com stat()
  {
   perror("lstat ERROR");
   exit(2);
  }
	if (S_ISREG(stat_buf.st_mode)){
		  if(strcmp(direntp->d_name, name)==0){
	  				//printf("%-25s\n", direntp->d_name);
		  }
		}
	  else
	  {
	  	if (S_ISDIR(stat_buf.st_mode)){
	  			if(strcmp(direntp->d_name,"..")>0)
	  				{
	  				printf("%-25s\n", direntp->d_name);
	  				//printName(name, direntp->d_name);
	  			}
	  	}
	  }
	}
}