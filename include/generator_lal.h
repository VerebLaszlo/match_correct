/**	@file   generator_lal.h
 *	@author László Veréb
 *	@date   3.09.2012
 *	@brief  Waveform generator.
 */

#ifndef GENERATOR_LAL_H_
#define GENERATOR_LAL_H_

#include <stdio.h>
#include "parser_confuse.h"

/** Various constants. */
enum {
	HP, HC, WAVE,
};

typedef struct {
	size_t length;
	double *h[WAVE];
	double *V;
	double *Phi;
	double *S1[DIMENSION];
	double *S2[DIMENSION];
	double *E1[DIMENSION];
	double *E3[DIMENSION];
} Output;

/**
 * Generates a waveform.
 * @param[in] parameter defining the waveform.
 * @param[in] output    generated output.
 * @return success code
 */
int generate(Parameter *parameter, Output *output);

/**
 * Frees the allocated memory.
 * @param[in] output memories to free.
 */
void cleanOutput(Output *output);

/**
 * Prints the generated values to a file.
 * @param[in] file      where to print.
 * @param[in] output    what to print.
 * @param[in] parameter the waveforms parameters.
 * @return success code
 */
int printOutput(FILE *file, Output *output, Parameter *parameter);

#endif /* GENERATOR_LAL_H_ */
