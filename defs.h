#ifndef DEFS
#define  DEFS

#include <ctype.h>	// isdigit()
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#endif
