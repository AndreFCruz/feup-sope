#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sigIntHandler(int signo);

int main(void)
{
	signal(SIGINT, sigIntHandler);

	exit(0);
}

void sigIntHandler(int signo){
	char ans;

	printf("Are you sure you want to terminate (Y/N)?\n");
	scanf("%c", &ans);

	if(ans == 'y' || ans == 'Y')
	{
  		exit(0);
	}
}