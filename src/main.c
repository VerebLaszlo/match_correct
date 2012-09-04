/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include <stdlib.h>
#include <string.h>
#include "generator_lal.h"
#include "util_IO.h"

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	char *input = argc > 1 ? argv[1] : "test.conf";
	Parameter parameter;
	memset(&parameter, 0, sizeof(Parameter));
	int success;
	success = initParser();
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	success = parse(input, &parameter);
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	printParameter(stdout, &parameter);
	Output output;
	memset(&output, 0, sizeof(Output));
	puts("A");
	generate(&parameter, &output);
	puts("B");
	FILE *file = safelyOpenForWriting("out/all.txt");
	puts("C");
	printOutput(file, &output, &parameter);
	puts("D");
	fclose(file);
	puts("E");
	cleanOutput(&output);
	puts("F");
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	puts("OK!");
}
