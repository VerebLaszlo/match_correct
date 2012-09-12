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
	int failure = SUCCESS;
	initParser();
	failure &= parse(input, &parameter);
	failure &= parseWaves(input, &parameter);
	if (!failure) {
		Variable *variable;
		for (size_t index = 0; index < parameter.length; index++) {
			variable = generateWaveformPair(&parameter.wave[2 * index], parameter.initialFrequency,
			        parameter.samplingTime);
			FILE *file = safelyOpenForWriting("out/all.txt");
			//failure &= printOutput(file, &output, &parameter.wave[0], parameter.samplingTime);
			fclose(file);
			destroyWaveform(&variable->wave);
			destroyOutput(&variable);
		}
	}
	cleanParameter(&parameter);
	if (!failure) {
		puts("OK!");
	} else {
		puts("Error!");
	}
}
