#ifndef HEADER
#define  HEADER

#include <ctype.h>	// isdigit()
#include <fcntl.h>  // O_CREAT, O_EXCL, etc
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> // mmap, munmap
#include <sys/stat.h> // open()
#include <sys/wait.h>
#include <unistd.h>

#define MMAP(p) {(p) = mmap(NULL, sizeof(*(p)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(p) {munmap((p), sizeof((p)));}

/* semaphore names */
#define SEM_APRINT "/xbucka00_sem1"

FILE *output = NULL;

/* error codes */
enum error_codes
{
    ECODE_SUCCESS = 0,
	ECODE_ERROR = 1
};

int NO = 0;		// number of Os
int NH = 0;		// number of Hs
int TI = 0;		// waiting time miliseconds
int TB = 0;		// mol creation time miliseconds

/* shared variables */
int *A_line_id = NULL; // A

/* semaphores */
sem_t *sem_aprint = NULL;

#endif
