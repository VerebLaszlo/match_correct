/**
 * @file lal_wrapper.h
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#ifndef LAL_WRAPPER_H_
#define LAL_WRAPPER_H_

#include "signals.h"
#include "parameters.h"

/**	Generates a waveform pair.
 * @param[in] parameters	   : the waveform's parameters
 * @param[out] signal		   : the generated waveform pair
 * @param[in] calculateMatches : true if you want to calculate match
 * @return
 */
int generateWaveformPair(SystemParameter *parameters, SignalStruct *signal, bool calculateMatches);

#endif /* LAL_WRAPPER_H_ */
