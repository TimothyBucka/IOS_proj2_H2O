#include "header.h"

/*
	Author: Timotej Bucka - xbucka00
	Year: 2022
	Solution inspiration: https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
*/


/*
Shared:
	All mol created
	Nof O
	Nof H
	Molecule id
	Molecule created
*/

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
 * 
 * @returns true - if arguments are correct
 */
bool arg_parse(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr, "Invalid number of arguments\n");
		return false;
	} else {
		if (is_integer(argv[1]) && is_integer(argv[2]) && is_integer(argv[3]) && is_integer(argv[4])) {
			NO = atoi(argv[1]);
			NH = atoi(argv[2]);
			TI = atoi(argv[3]);
			TB = atoi(argv[4]);
			if (NO > INT_MAX || NH > INT_MAX || TI > 1000 || TB > 1000) {
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

bool init_lab() {
	output = fopen("proj2.out", "w");
	if (output == NULL) {
		fprintf(stderr, "Unable to open .out file\n");
		return false;
	}
	setbuf(output, NULL);
	setbuf(stdout, NULL); //FIXME

	MMAP(A_line_id);
	*A_line_id = 1;
	MMAP(hydrogen_id);
	*hydrogen_id = 0;
	MMAP(oxygen_id);
	*oxygen_id = 0;
	MMAP(molecule_id);
	*molecule_id = 1;
	MMAP(hydrogen_mol_count);
	*hydrogen_mol_count = 0;
	MMAP(oxygen_mol_count);
	*oxygen_mol_count = 0;

	/* Barrier initialization*/
	MMAP(My_Barrier.n);
	*My_Barrier.n = WATER_ATOMS;
	SEM_INIT(My_Barrier.mutex, "/xbucka00.barrier_mutex", 1);
	if (My_Barrier.mutex == SEM_FAILED) return false;
	SEM_INIT(My_Barrier.turnstile1, "/xbucka00.barrier_turnstile1", 0);
	if (My_Barrier.turnstile1 == SEM_FAILED) return false;
	SEM_INIT(My_Barrier.turnstile2, "/xbucka00.barrier_turnstile2", 1);
	if (My_Barrier.turnstile2 == SEM_FAILED) return false;

	SEM_INIT(sem_line_print, "/xbucka00.sem_line_print", 1);
	if (sem_line_print == SEM_FAILED) return false;
	SEM_INIT(mutex, "/xbucka00.mutex", 1);
	if (mutex == SEM_FAILED) return false;
	SEM_INIT(hydrogen_queue, "/xbucka00.hydrogen_queue", 0);
	if (hydrogen_queue == SEM_FAILED) return false;
	SEM_INIT(oxygen_queue, "/xbucka00.oxygen_queue", 0);
	if (oxygen_queue == SEM_FAILED) return false;

	return true;
}

void clear_lab() {
	if (output != NULL) {
		fclose(output);
	}

	UNMAP(A_line_id);
	UNMAP(hydrogen_id);
	UNMAP(oxygen_id);
	UNMAP(molecule_id);
	UNMAP(hydrogen_mol_count);
	UNMAP(oxygen_mol_count);

	/* Barrier clearing */
	UNMAP(My_Barrier.n);
	SEM_CLEAR(My_Barrier.mutex, "/xbucka00.barrier_mutex");
	SEM_CLEAR(My_Barrier.turnstile1, "/xbucka00.barrier_turnstile1");
	SEM_CLEAR(My_Barrier.turnstile2, "/xbucka00.barrier_turnstile2");

	SEM_CLEAR(sem_line_print, "/xbucka00.sem_line_print");
	SEM_CLEAR(mutex, "/xbucka00.mutex");
	SEM_CLEAR(hydrogen_queue, "/xbucka00.hydrogen_queue");
	SEM_CLEAR(oxygen_queue, "/xbucka00.oxygen_queue");
}

void moleculing(char type, int atom_id) {
	sem_wait(sem_line_print);
			fprintf(output, "%d: %c %d: creating molecule %d\n", (*A_line_id)++, type, atom_id, *molecule_id);
	sem_post(sem_line_print);

	if (type == 'O') {
		usleep(1000*(rand()%(TB+1)));	// after sleep enter barrier
	}

	sem_wait(My_Barrier.mutex);
		(*My_Barrier.n)--;
		if (*My_Barrier.n == 0) {
			sem_post(My_Barrier.turnstile1);	// last atom signals turnstile that molecule is done
		}
	sem_post(My_Barrier.mutex);

	sem_wait(My_Barrier.turnstile1);
	sem_post(My_Barrier.turnstile1);

	sem_wait(sem_line_print);
			fprintf(output, "%d: %c %d: molecule %d created\n", (*A_line_id)++, type, atom_id, *molecule_id);
	sem_post(sem_line_print);

	sem_wait(My_Barrier.mutex);
		(*My_Barrier.n)++;
		if (*My_Barrier.n == WATER_ATOMS) {
			(*molecule_id)++;
			sem_wait(My_Barrier.turnstile1);
			sem_post(mutex);	// last atom opens mutex for other molecules
		}
	sem_post(My_Barrier.mutex);
}

void hydrogen(int atom_id) {
	sem_wait(sem_line_print);
		fprintf(output, "%d: H %d: started\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);
	
	srand(time(NULL)*getpid());
	usleep(1000*(rand()%(TI+1)));

	sem_wait(sem_line_print);
		fprintf(output, "%d: H %d: going to queue\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	sem_wait(mutex);
	(*hydrogen_id)++;
	if ((NO<<1 < NH && *hydrogen_id >= (NO<<1)+1) || (NO<<1 > NH && NH&1 && *hydrogen_id==NH)) {
		sem_wait(sem_line_print);
			fprintf(output, "%d: H %d: not enough O or H\n", (*A_line_id)++, atom_id);
		sem_post(sem_line_print);
		sem_post(mutex);
		return;
	} else {
		(*hydrogen_mol_count)++;
		if (*hydrogen_mol_count >= 2 && *oxygen_mol_count >= 1) {
			sem_post(hydrogen_queue);
			sem_post(hydrogen_queue);
			*hydrogen_mol_count -= 2;
			sem_post(oxygen_queue);
			(*oxygen_mol_count)--;
		} else {
			sem_post(mutex);
		}

		sem_wait(hydrogen_queue);

		moleculing('H', atom_id);
	}
}

void oxygen(int atom_id) {
	sem_wait(sem_line_print);
		fprintf(output, "%d: O %d: started\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	srand(time(NULL)*getpid());
	usleep(1000*(rand()%(TI+1)));

	sem_wait(sem_line_print);
		fprintf(output, "%d: O %d: going to queue\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	sem_wait(mutex);
	(*oxygen_id)++;
	if (NO<<1 >= NH && *oxygen_id >= ((NH>>1) + 1)) {
		sem_wait(sem_line_print);
			fprintf(output, "%d: O %d: not enough H\n", (*A_line_id)++, atom_id);
		sem_post(sem_line_print);
		sem_post(mutex);
		return;
	} else {
		(*oxygen_mol_count)++;
		if (*hydrogen_mol_count >= 2) {
			sem_post(hydrogen_queue);
			sem_post(hydrogen_queue);
			*hydrogen_mol_count -= 2;
			sem_post(oxygen_queue);
			(*oxygen_mol_count)--;
		} else {
			sem_post(mutex);
		}

		sem_wait(oxygen_queue);

		moleculing('O', atom_id);
	}
}

void gen_hydrogen() {
	for (int i=1; i<=NH; i++) {
		pid_t h_id = fork();
		if (h_id == 0) {
			hydrogen(i);
			exit(EXIT_SUCCESS);
		} else if (h_id < 0) {
			fprintf(stderr, "Unable to fork\n");
			clear_lab();
			exit(EXIT_FAILURE);
		}
	}
}

void gen_oxygen() {
	for (int i=1; i<=NO; i++) {
		pid_t o_id = fork();
		if (o_id == 0) {
			oxygen(i);
			exit(EXIT_SUCCESS);
		} else if (o_id < 0) {
			fprintf(stderr, "Unable to fork\n");
			clear_lab();
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[]){
	if (!arg_parse(argc, argv)) 
		return ECODE_ERROR;
	
	if (!init_lab()) {
		clear_lab();
		return ECODE_ERROR;
	}

	gen_hydrogen();
	gen_oxygen();

	while (wait(NULL) > 0);
	
   	clear_lab();
   	return ECODE_SUCCESS;
}
