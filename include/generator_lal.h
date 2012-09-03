/**	@file   generator_lal.h
 *	@author László Veréb
 *	@date   3.09.2012
 *	@brief  Waveform generator.
 */

#ifndef GENERATOR_LAL_H_
#define GENERATOR_LAL_H_

#include <stdio.h>
#include <lal/LALConstants.h>
#include <lal/LALDatatypes.h>
#include <lal/LALSimInspiral.h>
#include <lal/TimeSeries.h>
#include "parser_confuse.h"

/** Various constants. */
enum {
	HP, HC, WAVE,
};

/** Structure containing the output vectors. */
typedef struct {
	REAL8TimeSeries *h[WAVE];
	REAL8TimeSeries *V;
	REAL8TimeSeries *Phi;
	REAL8TimeSeries *S1[DIMENSION];
	REAL8TimeSeries *S2[DIMENSION];
	REAL8TimeSeries *E1[DIMENSION];
	REAL8TimeSeries *E3[DIMENSION];
} Output;

/**
 * Prints the generated values to a file.
 * @param[in] file   where to print.
 * @param[in] output what to print.
 * @param[in] dt     sampling time.
 * @return success code
 */
int printOutput(FILE *file, Output *output, double dt);

/**
 * Generates a waveform.
 * @param[in] parameter defining the waveform.
 * @return success code
 */
int generate(Parameter *parameter);

#endif /* GENERATOR_LAL_H_ */
