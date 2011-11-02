/**
 * @file parser.h
 * @author vereb
 * @date Sep 28, 2011
 * @brief 
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "parameters.h"

Limits *createWaveformPairLimitsFrom(cstring fileName, ConstantParameters *constants,
	size_t *numberOfPairs);

void destroyWaveformPairLimits(Limits *limits);

Limits *createSignalAndTemplatesLimitsFrom(cstring fileName, ConstantParameters *constants,
	size_t *size);

SystemParameter *createExactWaveformPairFrom(cstring fileName, size_t *numberOfPairs);

void destroyExactWaveformPairs(SystemParameter *pairs);

void destroySignalAndTemplatesLimits(Limits *limits);

void getProgramParametersFrom(cstring fileName, ProgramParameter *parameters, Options *option);

#endif /* PARSER_H_ */
