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
	HP, HC, WAVE, FIRST = 0, SECOND, NUMBER_OF_WAVES,
};

typedef struct {
	double *h[NUMBER_OF_WAVES][WAVE];
	double *V[NUMBER_OF_WAVES];
	double *Phi[NUMBER_OF_WAVES];
	double *S1[NUMBER_OF_WAVES][DIMENSION];
	double *S2[NUMBER_OF_WAVES][DIMENSION];
	double *E1[NUMBER_OF_WAVES][DIMENSION];
	double *E3[NUMBER_OF_WAVES][DIMENSION];
	size_t length[NUMBER_OF_WAVES];
	size_t size;
} Output;

/**
 * Generates a waveform.
 * @param[in] wave             waveform parameters.
 * @param[in] output           generated output.
 * @param[in] initialFrequency starting frequency
 * @param[in] samplingTime     sampling time
 * @return failure code
 */
int generateWaveformPair(Wave wave[], double initialFrequency, double samplingTime, Output *output);

/**
 * Frees the allocated memory.
 * @param[in] output memories to free.
 */
void cleanOutput(Output *output);

/**
 * Prints the generated values to a file.
 * @param[in] file         where to print.
 * @param[in] output       what to print.
 * @param[in] wave         the waveforms parameters.
 * @param[in] samplingTime sampling time
 * @return success code
 */
int printOutput(FILE *file, Output *output, Wave *wave, double samplingTime);

#endif /* GENERATOR_LAL_H_ */
