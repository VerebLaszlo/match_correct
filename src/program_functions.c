/**
 * @file program_functions.c
 * @author vereb
 * @date Sep 27, 2011
 * @brief 
 */

#include <math.h>
#include "program_functions.h"
#include "parser.h"
#include "match.h"
#include "parser.h"
#include "lal_wrapper.h"

void runProgram(cstring programFileName, cstring parameterFileName, Options *option) {
	ProgramParameter program;
	getProgramParametersFrom(programFileName, &program, option);
	runForWaveformPairs(parameterFileName, &program);
	runForSignalAndTemplates(parameterFileName, &program);
}

void runForSignalAndTemplates(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	SystemParameter parameter;
	size_t numberOfTemplatesWithSignal;
	Limits *template = createSignalAndTemplatesLimitsFrom(fileName, &constants,
		&numberOfTemplatesWithSignal);
	if (numberOfTemplatesWithSignal) {
		getSysemParametersFromLimit(template, &constants, &parameter, 0);
		for (ushort currentTemplate = 1; currentTemplate < numberOfTemplatesWithSignal;
			currentTemplate++) {
			getSysemParametersFromLimit(template, &constants, &parameter, 1);
			run(program, &parameter);
		}
	}
	destroySignalAndTemplatesLimits(template);
}

void runForWaveformPairs(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	size_t numberOfPairs;
	Limits *pair = createWaveformPairLimitsFrom(fileName, &constants, &numberOfPairs);
	if (numberOfPairs) {
		for (ushort currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			SystemParameter parameter;
			getSysemParametersFromLimits(&pair[currentPair], &constants, &parameter);
			run(program, &parameter);
		}
	}
	destroyWaveformPairLimits(pair);
}

/**	Runs the program.
 * @param[in] program	   :
 * @param[in] parameters   :
 */
void run(ProgramParameter *program, SystemParameter *parameters) {
	setSignalExistanceFunctions(program->calculateMatches);
	SignalStruct signal;
	generateWaveformPair(parameters, &signal, program->calculateMatches);
	double match[NUMBER_OF_MATCHES] = { 0.0, 0.0, 0.0 };
	if (program->calculateMatches) {
		size_t min, max;
		calculateIndexBoundariesFromFrequencies(parameters->initialFrequency,
			parameters->endingFrequency, parameters->samplingFrequency, &min, &max);
		calc_Matches(&signal, min, max, &match[TYPICAL], &match[BEST], &match[WORST]);
	}
	if (program->plot) {
		for (size_t i = 0; i < signal.size; i++) {
			signal.inTime[H1][i] = M_SQRT1_2
				* (signal.componentsInTime[H1P][i] + signal.componentsInTime[H1C][i]);
			signal.inTime[H2][i] = M_SQRT1_2
				* (signal.componentsInTime[H2P][i] + signal.componentsInTime[H2C][i]);
		}
		char fileName[1000];
		sprintf(fileName, "%s/%s", program->outputDirectory, "proba.dat");
		FILE *file = safelyOpenForWriting(fileName);
		printParametersForSignalPlotting(file, parameters, match, defaultFormat);
		printTwoSignals(file, &signal, defaultFormat);
		fclose(file);
	}
	destroySignal(&signal);
}

/**	Calls the testing functions.
 * @return succes of the run.
 */bool testingFunctions(void) {
	srand(86);
	bool succes = true;
	if (!areUtilMathFunctionsOK()) {
		succes = false;
	} else if (!areIOFunctionsGood()) {
		succes = false;
	} else if (!areBinarySystemMassFunctionsGood()) {
		succes = false;
	} else if (!areBinarySystemSpinFunctionsGood()) {
		succes = false;
	} else if (!areBinarySystemFunctionsGood()) {
		succes = false;
	} else if (!areDetectorFunctionsGood()) {
		succes = false;
	}
	return succes;
}
