/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser_confuse.h"

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	char *file = argc > 1 ? argv[1] : "test.conf";
	Parameter parameters;
	int success = parse(file, &parameters);
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	printParameter(stdout, &parameters);
	puts("OK!");
}
