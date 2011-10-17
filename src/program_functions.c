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
			for (size_t templateMultiply = 0; templateMultiply < template[currentTemplate].number;
				templateMultiply++) {
				getSysemParametersFromLimit(template, &constants, &parameter, 1);
				run(program, &parameter, templateMultiply);
			}
		}
	}
	destroySignalAndTemplatesLimits(template);
}

void runForWaveformPairs(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	size_t numberOfPairs;
	Limits *pair = createWaveformPairLimitsFrom(fileName, &constants, &numberOfPairs);
	if (numberOfPairs) {
		for (size_t currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			for (size_t pairMultiply = 0; pairMultiply < pair[currentPair].number; pairMultiply++) {
				SystemParameter parameter;
				getSysemParametersFromLimits(&pair[currentPair], &constants, &parameter);
				run(program, &parameter, pairMultiply);
			}
		}
	}
	destroyWaveformPairLimits(pair);
}

/**	Runs the program.
 * @param[in] program	   :
 * @param[in] parameters   :
 */
void run(ProgramParameter *program, SystemParameter *parameters, size_t number) {
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
		sprintf(fileName, "%s/%s.%02d.dat", program->outputDirectory, "proba", number);
		FILE *file = safelyOpenForWriting(fileName);
		printParametersForSignalPlotting(file, parameters, match);
		printTwoSignals(file, &signal);
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
