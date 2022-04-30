#include "header.h"

/*
	Author: Timotej Bucka - xbucka00
	Year: 2022
	Solution inspiration: https://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
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

/**
 * @brief Open output folder, map shared memory, open semaphores
 * 
 * @return flase - if error occurs
 */
bool init_lab() {
	output = fopen("proj2.out", "w");
	if (output == NULL) {
		fprintf(stderr, "Unable to open .out file\n");
		return false;
	}
	setbuf(output, NULL);

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

/**
 * @brief Close output folder, unmap shared memory, close and unlink semaphores
 * 
 */
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


/**
 * @brief Runs when processes create  watermolecule
 * 
 * @param type Is either H or O depending on which process calls it
 * @param atom_id Id of each atom, used for print
 */
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

/**
 * @brief Process each hydrogen atom executes
 * 
 * @param atom_id Identificator for each hydrogen process
 */
void hydrogen(int atom_id) {
	sem_wait(sem_line_print);
		fprintf(output, "%d: H %d: started\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);
	
	srand(time(NULL)*getpid());
	usleep(1000*(rand()%(TI+1)));	// time for entering the queue

	sem_wait(sem_line_print);
		fprintf(output, "%d: H %d: going to queue\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	sem_wait(mutex);	// only one process enters
	(*hydrogen_id)++;
	// check if a hydrogen atom is extra (has no O to bond with) or O is missing a hydrogen to make molecule with
	if ((NO<<1 < NH && *hydrogen_id >= (NO<<1)+1) || (NO<<1 > NH && NH&1 && *hydrogen_id==NH)) {
		sem_wait(sem_line_print);
			fprintf(output, "%d: H %d: not enough O or H\n", (*A_line_id)++, atom_id);
		sem_post(sem_line_print);
		sem_post(mutex);	// open mutex for other processes to enter
		return;	// after return dies
	} else {
		(*hydrogen_mol_count)++;
		if (*hydrogen_mol_count >= 2 && *oxygen_mol_count >= 1) { // if the right amount of each type of process is reached
			sem_post(hydrogen_queue); // let 2 (waiting) hydrogen processes continue
			sem_post(hydrogen_queue);
			*hydrogen_mol_count -= 2; // reset count
			sem_post(oxygen_queue);	// let 1 (waiting) oxygen process continue
			(*oxygen_mol_count)--;	// reset count
		} else {
			sem_post(mutex); // open mutex for other processes to enter
		}

		sem_wait(hydrogen_queue); // wait until right amount

		moleculing('H', atom_id); // create molecule
	}
}

/**
 * @brief Process each oxygen atom executes
 * 
 * @param atom_id Identificator for each oxygen process
 */
void oxygen(int atom_id) {
	sem_wait(sem_line_print);
		fprintf(output, "%d: O %d: started\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	srand(time(NULL)*getpid());
	usleep(1000*(rand()%(TI+1))); // time for entering the queue

	sem_wait(sem_line_print);
		fprintf(output, "%d: O %d: going to queue\n", (*A_line_id)++, atom_id);
	sem_post(sem_line_print);

	sem_wait(mutex); // only one process enters
	(*oxygen_id)++;
	// check if there is more oxygens than there should be considering the amount of hydrogens (Am I missing a hydrogen?)
	if (NO<<1 > NH && *oxygen_id >= ((NH>>1) + 1)) {
		sem_wait(sem_line_print);
			fprintf(output, "%d: O %d: not enough H\n", (*A_line_id)++, atom_id);
		sem_post(sem_line_print);
		sem_post(mutex); // open mutex for other processes to enter
		return; // dies
	} else {
		(*oxygen_mol_count)++;
		if (*hydrogen_mol_count >= 2) { // i have one oxygen, have I got 2 hydrogens?
			sem_post(hydrogen_queue); // let 2 (waiting) hydrogens continue
			sem_post(hydrogen_queue);
			*hydrogen_mol_count -= 2; // reset hydrogens needed
			sem_post(oxygen_queue); // let 1 (waiting) oxygen continue
			(*oxygen_mol_count)--; // reset oxygens needed
		} else {
			sem_post(mutex); // let another process
		}

		sem_wait(oxygen_queue);	// wait until right amount

		moleculing('O', atom_id); // create molecule
	}
}


/**
 * @brief Creates NH number of processes aka hydrogens
 * 
 */
void gen_hydrogen() {
	for (int i=1; i<=NH; i++) {
		pid_t h_id = fork();
		if (h_id == 0) {
			hydrogen(i);
			exit(EXIT_SUCCESS); // dies after executing hydrogen()
		} else if (h_id < 0) {
			fprintf(stderr, "Unable to fork\n");
			clear_lab();
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * @brief Creates NO number of processes aka oxygens
 * 
 */
void gen_oxygen() {
	for (int i=1; i<=NO; i++) {
		pid_t o_id = fork();
		if (o_id == 0) {
			oxygen(i);
			exit(EXIT_SUCCESS); // dies after executing oxygen()
		} else if (o_id < 0) {
			fprintf(stderr, "Unable to fork\n");
			clear_lab();
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * @brief Main function of the program
 * 
 * @param argc Numer of arguments including executable name
 * @param argv Pointer to array of characters representing programs arguments
 * @return returns EXIT_FAILURE if error occurs or returns EXIT_SUCCESS if everything went well
 */
int main(int argc, char *argv[]){
	if (!arg_parse(argc, argv)) 
		return EXIT_FAILURE;
	
	if (!init_lab()) { // something goes wrong in initialization
		clear_lab();
		return EXIT_FAILURE;
	}

	// generate processes
	gen_hydrogen();
	gen_oxygen();

	while (wait(NULL) > 0);	// wait for all child processes to die
	
   	clear_lab(); // clear allocated memory
   	return EXIT_SUCCESS;
}
