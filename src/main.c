/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include <stdlib.h>
#include <string.h>
#include "generator_lal.h"

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	char *file = argc > 1 ? argv[1] : "test.conf";
	Parameter parameter;
	memset(&parameter, 0, sizeof(Parameter));
	int success = parse(file, &parameter);
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	printParameter(stdout, &parameter);
	generate(&parameter);
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	puts("OK!");
}
