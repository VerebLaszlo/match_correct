/**
 * @file main_test.c
 *
 * @date Jul 30, 2011
 * @author vereb
 * @brief
 */

#include "program_functions.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static void printHelp(void) {
	puts("h:              prints this help");
	puts("p:              save the generated waveform for plotting");
	puts("m:              calculate the match");
	puts("s:              the file containing the parameters");
	puts("i:              the file initialize the program");
	puts("e:              read exact parameters");
	puts("n:              number of runs between time measurements");
	puts("c:              copy the generated parameters from the first system to the second.\n"
		"    It is good to compare various approximants.");
	puts("d_ step1 step2: to give the number of the steps. It also sets the copy function.\n"
		"    The character _ can be one of the following:\n"
		"      m for total Mass (M) and symmetric mass ratio (eta),\n"
		"      s for spin parameter (chi1, chi2),\n"
		"      i for inclinations (kappa1, kappa2),\n"
		"      a for azimuths (beta1, beta2.\n");
	puts("t:              testing");
}

static void interpretOptions(Options *option, int argc, char *argv[]) {
	memset(option, 0, sizeof(Options));
	while (argc) {
		if ((*argv)[0] == '-') {
			if ((*argv)[1] == 'h') {
				printHelp();
				exit(EXIT_SUCCESS);
			} else if ((*argv)[1] == 'p') {
				option->plot = true;
			} else if ((*argv)[1] == 'm') {
				option->calculateMatch = true;
			} else if ((*argv)[1] == 'e') {
				option->exact = true;
			} else if ((*argv)[1] == 'c') {
				option->copy = true;
			} else if ((*argv)[1] == 'n') {
				argc--;
				argv++;
				option->stepSize = strtoul(*argv, NULL, 10);
			} else if ((*argv)[1] == 's') {
				argc--;
				argv++;
				strcpy(option->parameterFile, *argv);
			} else if ((*argv)[1] == 'i') {
				argc--;
				argv++;
				strcpy(option->programFile, *argv);
			} else if ((*argv)[1] == 'd') {
				option->copy = true;
				if ((*argv)[2] == 'm') {
					option->step.massSet = true;
				} else if ((*argv)[2] == 's') {
					option->step.chiSet = true;
				} else if ((*argv)[2] == 'i') {
					option->step.inclSet = true;
				} else if ((*argv)[2] == 'a') {
					option->step.azimSet = true;
				} else {
					fprintf(stderr, "Bad stepping option, parameter could not be stepped: %c\n",
						(*argv)[2]);
					exit(-1);
				}
				argc--;
				argv++;
				ushort step = strtoul(*argv, NULL, 10);
				if (step <= 0.0 || step == fabs(HUGE_VAL)) {
					fprintf(stderr, "Bad stepping value, need to be positive: %u\n", step);
					perror("The following error occurred");
					exit(-1);
				}
				option->step.step[0] = step;
				argc--;
				argv++;
				step = strtoul(*argv, NULL, 10);
				if (step <= 0.0 || step == fabs(HUGE_VAL)) {
					fprintf(stderr, "Bad stepping value, need to be positive: %u\n", step);
					perror("The following error occurred");
					exit(-1);
				}
				option->step.step[1] = step;
				option->step.set = true;
			} else if ((*argv)[1] == 't') {
				option->testing = true;
			}
		}
		argc--;
		argv++;
	}
}

int main(int argc, char *argv[]) {
	Options option;
	if (argc == 1) {
		printHelp();
		exit(EXIT_SUCCESS);
	}
	argc--;
	argv++;
	interpretOptions(&option, argc, argv);
	if (option.testing) {
#ifdef TEST
		testingFunctions();
#endif // TEST
	} else {
		runProgram(option.programFile, option.parameterFile, &option);
	}
	puts("\nOK");
	return 0;
}
