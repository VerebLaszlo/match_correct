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

/**	Runs the program.
 * @param[in] program	   :
 * @param[in] parameters   :
 */
static void run(ProgramParameter *program, SystemParameter *parameters, size_t number) {
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
		sprintf(fileName, "%s/%s.%02d.dat", program->outputDirectory, parameters->name[1], number);
		FILE *file = safelyOpenForWriting(fileName);
		printParametersForSignalPlotting(file, parameters, match);
		printTwoSignals(file, &signal);
		fclose(file);
	}
	destroySignal(&signal);
}

static void runForSignalAndTemplates(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	SystemParameter parameter;
	size_t numberOfTemplatesWithSignal;
	Limits *template = createSignalAndTemplatesLimitsFrom(fileName, &constants,
		&numberOfTemplatesWithSignal);
	if (numberOfTemplatesWithSignal) {
		getSysemParametersFromLimit(template, &constants, &parameter, 0);
		for (ushort currentTemplate = 1; currentTemplate < numberOfTemplatesWithSignal;
			currentTemplate++) {
			for (size_t templateMultiply = 0;
				templateMultiply < template[currentTemplate].numberOfRuns; templateMultiply++) {
				getSysemParametersFromLimit(&template[currentTemplate], &constants, &parameter, 1);
				run(program, &parameter, templateMultiply);
			}
		}
	}
	destroySignalAndTemplatesLimits(template);
}

static void runForWaveformPairs(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	size_t numberOfPairs;
	Limits *pair = createWaveformPairLimitsFrom(fileName, &constants, &numberOfPairs);
	string configName;
	sprintf(configName, "%s/data_%s", program->outputDirectory, fileName);
	FILE *file = safelyOpenForWriting(configName);
	printStartOfConfigFile(file, &constants);
	if (numberOfPairs) {
		for (size_t currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			for (size_t pairMultiply = 0; pairMultiply < pair[2 * currentPair + 1].numberOfRuns;
				pairMultiply++) {
				SystemParameter parameter;
				getSysemParametersFromLimits(&pair[2 * currentPair], &constants, &parameter);
				run(program, &parameter, pairMultiply);
				printWaveformPairsToConfigFile(file, &parameter, &outputFormat[DATA]);
				if ((currentPair != numberOfPairs - 1)
					|| pairMultiply != pair[2 * currentPair + 1].numberOfRuns - 1) {
					printMiddleOfConfigFile(file);
				}
			}
		}
	}
	printEndOfConfigFile(file);
	fclose(file);
	destroyWaveformPairLimits(pair);
}

static void runForExactWaveformPairs(cstring fileName, ProgramParameter *program) {
	ConstantParameters constants;
	size_t numberOfPairs;
	SystemParameter *pair = createExactWaveformPairFrom(fileName, &constants, &numberOfPairs);
	if (numberOfPairs) {
		for (size_t currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
				convertBinarySystemParameters(&pair->system[i]);
			}
			run(program, pair, 0);
		}
	}
	destroyExactWaveformPairs(pair);
}

void runProgram(cstring programFileName, cstring parameterFileName, Options *option) {
	ProgramParameter program;
	getProgramParametersFrom(programFileName, &program, option);
	if (option->exact) {
		runForExactWaveformPairs(parameterFileName, &program);
	} else {
		runForWaveformPairs(parameterFileName, &program);
		runForSignalAndTemplates(parameterFileName, &program);
	}
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
