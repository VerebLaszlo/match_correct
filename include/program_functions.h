/**
 * @file program_functions.h
 * @author vereb
 * @date Sep 27, 2011
 * @brief 
 */

#ifndef PROGRAM_FUNCTIONS_H_
#define PROGRAM_FUNCTIONS_H_

#include "parameters.h"

void runProgram(cstring programFileName, cstring parameterFileName, Options *option);

void runForSignalAndTemplates(cstring fileName, ProgramParameter *program);

void runForWaveformPairs(cstring fileName, ProgramParameter *program);

void run(ProgramParameter *program, SystemParameter *parameters, size_t number);

bool testingFunctions(void);

#endif /* PROGRAM_FUNCTIONS_H_ */
