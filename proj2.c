/*
	How to run the program?

	$ ./proj2 NO NH TI TB
	NO - number of oxygens
	NH - number of hydrogens
	TI - max time (miliseconds) for which an atom of O/H waits after its creation
		 to join the queue for molecule creation 0 <= TI <= 1000
	TB - max time (miliseconds) needed for one molecule creation 0 <= TB <= 1000
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* error codes */
enum error_codes
{
    ECODE_SUCCESS = 0,
	ECODE_ERROR = 1
};

sem_t my_sem;



int main(int argc, char *argv[]){
	(void)argv;
	(void)argc;

	int pid_O = 0;	// child process 1
	int pid_H = 0;	// child process 2

	pid_O = fork();	// create O

	if (pid_O < 0) {
		// smt went wrong
	} else if (pid_O == 0) {
		// O process code
		printf("ID: %d, PID: %d -- This is Oxygen Jesse\n", getpid(), getppid());
	} else {
		pid_H = fork();	// create H
		if (pid_H < 0) {
			// smt went wrong
		} else if (pid_H == 0) {
			// H process code
			printf("ID: %d, PID: %d -- This is Hydrogen Jesse\n", getpid(), getppid());
		} else {
			// parent process
			printf("ID: %d, PID: %d -- I am the parental danger Skyler\n", getpid(), getppid());
		}
	}
   
   return ECODE_SUCCESS;
}
