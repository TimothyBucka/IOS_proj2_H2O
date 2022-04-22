/*
	How to run the program?

	$ ./proj2 NO NH TI TB
	NO - number of oxygens
	NH - number of hydrogens
	TI - max time (miliseconds) for which an atom of O/H waits after its creation
		 to join the queue for molecule creation 0 <= TI <= 1000
	TB - max time (miliseconds) needed for one molecule creation 0 <= TB <= 1000
*/

#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* error codes */
enum error_codes
{
    ECODE_SUCCESS = 0,
	ECODE_ERROR = 1
};

/**
 * @brief Checks if string is a whole non-negative number
 * 
 * @param s input string
 */
bool is_integer(char *s) {
	for (int i=0; s[i] != '\0'; i++)
		if (!isdigit(s[i]))
			return false;
	return true;
}

/**
 * @brief Parses arguments and checks theirs validity
 * 
 * @param argc number of program arguments
 * @param argv list of pointers to arguments strings
 * @param NO pointer to number of oxygens
 * @param NH pointer to number of hydrogens
 * @param TI pointer to waiting time in miliseconds
 * @param TB pointer to molecule creation time in miliseconds
 * 
 * @returns true - if arguments are correct
 */
bool arg_parse(int argc, char **argv, int *NO, int *NH, int *TI, int *TB) {
	if (argc != 5) {
		fprintf(stderr, "Invalid number of arguments\n");
		return false;
	} else {
		if (is_integer(argv[1]) && is_integer(argv[2]) && is_integer(argv[3]) && is_integer(argv[4])) {
			*NO = atoi(argv[1]);
			*NH = atoi(argv[2]);
			*TI = atoi(argv[3]);
			*TB = atoi(argv[4]);
			if (*NO > INT_MAX || *NH > INT_MAX || *TI > 1000 || *TB > 1000) {
				fprintf(stderr, "Invalid value of arguments\n");
				return false;
			}
		} else {
			fprintf(stderr, "Invalid format of arguments\n");
			return false;
		}
	}
	return true;
}

int main(int argc, char *argv[]){

	int pid_O = 0;	// child process 1
	int pid_H = 0;	// child process 2
	int NO = 0;		// number of Os
	int NH = 0;		// number of Hs
	int TI = 0;		// waiting time miliseconds
	int TB = 0;		// mol creation time miliseconds

	if (!arg_parse(argc, argv, &NO, &NH, &TI, &TB)) 
		return ECODE_ERROR;

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
