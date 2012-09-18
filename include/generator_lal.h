/**	@file   generator_lal.h
 *	@author László Veréb
 *	@date   3.09.2012
 *	@brief  Waveform generator.
 */

#ifndef GENERATOR_LAL_H_
#define GENERATOR_LAL_H_

#include <stdio.h>
#include "parser_confuse.h"
#include "match_fftw.h"

typedef struct {
	Waveform *wave;
	double *V[NUMBER_OF_WAVE];
	double *Phi[NUMBER_OF_WAVE];
	double *S1[NUMBER_OF_WAVE][DIMENSION];
	double *S2[NUMBER_OF_WAVE][DIMENSION];
	double *E1[NUMBER_OF_WAVE][DIMENSION];
	double *E3[NUMBER_OF_WAVE][DIMENSION];
	size_t length[NUMBER_OF_WAVE];
	size_t size;
} Variable;

/**
 * Generates a waveform.
 * @param[in] wave             waveform parameters.
 * @param[in] output           generated output.
 * @param[in] initialFrequency starting frequency
 * @param[in] samplingTime     sampling time
 * @return failure code
 */
Variable* generateWaveformPair(Wave parameter[], double initialFrequency, double samplingTime);

/**
 * Frees the allocated memory.
 * @param[in] output memories to free.
 */
void destroyOutput(Variable **output);

void printSpins(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime);

void printSystem(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime);
/**
 * Prints the generated values to a file.
 * @param[in] file         where to print.
 * @param[in] output       what to print.
 * @param[in] wave         the waveforms parameters.
 * @param[in] samplingTime sampling time
 * @return success code
 */
int printOutput(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime);

#endif /* GENERATOR_LAL_H_ */
