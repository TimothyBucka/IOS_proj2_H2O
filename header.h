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
#include <time.h>
#include <unistd.h>

#define WATER_ATOMS 3

#define MMAP(p) {p = mmap(NULL, sizeof(*p), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(p) {munmap(p, sizeof(p));}

#define SEM_INIT(semaphore, name, value) {semaphore = sem_open(name, O_CREAT | O_EXCL, 0666, value);}
#define SEM_CLEAR(semaphore, name) {sem_close(semaphore); sem_unlink(name);}

FILE *output = NULL;

typedef struct {
    int *n; // number of atoms in water molecule (3)
    sem_t *mutex; // mutual exclution semaphore for entering barrier
    sem_t *turnstile1;
    sem_t *turnstile2;
} Barrier;

int NO = 0;		// number of Os
int NH = 0;		// number of Hs
int TI = 0;		// waiting time miliseconds
int TB = 0;		// mol creation time miliseconds

/* shared variables */
int *A_line_id = NULL; // A
int *hydrogen_id = NULL; // counts hydrogen processes
int *oxygen_id = NULL; // counts oxygen processes
int *molecule_id = NULL; // counts number of molecules
int *hydrogen_mol_count = NULL; // counts number of hydrogen processes needed for molecule
int *oxygen_mol_count = NULL; // counts number of oxygen processes needed for molecule

Barrier My_Barrier;

/* semaphores */
sem_t *sem_line_print = NULL; // for mutual exclusive access to output file
sem_t *mutex = NULL; // for only one process to execute code
sem_t *hydrogen_queue = NULL; // hydrogen wait for enough atoms to create molecule
sem_t *oxygen_queue = NULL; // oxygen wait for enough atoms to create molecule

#endif
