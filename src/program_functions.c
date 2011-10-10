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

void runForSignalAndTemplates(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	Limits *template;
	SystemParameter parameter;
	size_t numberOfTemplatesWithSignal = getSignalAndTemplatesLimitsFrom(fileName, &constants,
		&template);
	getSysemParametersFromLimit(template, &constants, &parameter, 0);
	for (ushort currentTemplate = 1; currentTemplate < numberOfTemplatesWithSignal;
		currentTemplate++) {
		getSysemParametersFromLimit(template, &constants, &parameter, 1);
		run(program, &parameter);
	}
	free(template);
}

void runForWaveformPairs(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	Limits *pair;
	size_t numberOfPairs = getWaveformPairLimitsFrom(fileName, &constants, &pair);
	for (ushort currentPair = 0; currentPair < numberOfPairs; currentPair++) {
		SystemParameter parameter;
		getSysemParametersFromLimits(&pair[currentPair], &constants, &parameter);
		run(program, &parameter);
	}
	free(pair);
}

/**	Runs the program.
 * @param[in] program	   :
 * @param[in] parameters   :
 */
void run(ProgramParameter *program, SystemParameter *parameters) {
	setSignalExistanceFunctions(program->calculateMatches);
	SignalStruct signal;
	generateWaveformPair(parameters, &signal, program->calculateMatches);
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
		printTwoSignals(file, &signal, defaultFormat);
		fclose(file);
	}
	if (program->calculateMatches) {
		double type, minimax, best;
		size_t min, max;
		calculateIndexBoundariesFromFrequencies(parameters->initialFrequency,
			parameters->endingFrequency, parameters->samplingFrequency, &min, &max);
		calc_Matches(&signal, min, max, &type, &best, &minimax);
		printf("%lg %lg %lg\n", minimax, type, best);
	}
	destroySignal(&signal);
}

/**	Calls the testing functions.
 * @return succes of the run.
 */
bool testingFunctions(void) {
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
