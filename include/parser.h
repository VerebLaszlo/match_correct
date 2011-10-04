/**
 * @file parser.h
 * @author vereb
 * @date Sep 28, 2011
 * @brief 
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "parameters.h"

size_t getWaveformPairLimitsFrom(cstring fileName, ConstantParameters *constants, Limits **pairsLimit);

size_t getSignalAndTemplatesLimitsFrom(cstring fileName, ConstantParameters *constants, Limits **waveformLimit);

#endif /* PARSER_H_ */
