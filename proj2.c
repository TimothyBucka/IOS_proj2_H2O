#include "defs.h"

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
	return true;
}

void clear_lab() {
	if (output != NULL) {
		fclose(output);
	}
}

int main(int argc, char *argv[]){
	if (!arg_parse(argc, argv)) 
		return ECODE_ERROR;
	
	if (!init_lab()) {
		clear_lab();
		return ECODE_ERROR;
	}
   
   clear_lab();
   return ECODE_SUCCESS;
}
