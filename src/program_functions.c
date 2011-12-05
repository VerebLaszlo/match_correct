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
	if (program->calculateMatches) {
		size_t min, max;
		calculateIndexBoundariesFromFrequencies(parameters->initialFrequency,
			parameters->endingFrequency, parameters->samplingFrequency, &min, &max);
		calc_Matches(&signal, min, max, &parameters->match[TYPICAL], &parameters->match[BEST],
			&parameters->match[WORST]);
	}
	if (program->plot) {
		for (size_t i = 0; i < signal.size; i++) {
			signal.inTime[H1][i] = M_SQRT1_2
				* (signal.componentsInTime[H1P][i] + signal.componentsInTime[H1C][i]);
			signal.inTime[H2][i] = M_SQRT1_2
				* (signal.componentsInTime[H2P][i] + signal.componentsInTime[H2C][i]);
		}
		char fileName[1000];
		sprintf(fileName, "%s/%s%02d.dat", program->outputDirectory, parameters->name[1], number);
		FILE *file = safelyOpenForWriting(fileName);
		printParametersForSignalPlotting(file, parameters, parameters->match);
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

static void runWithStep(cstring fileName, Step *steps, bool copy, ProgramParameter *program) {
	size_t numberOfPairs;
	ConstantParameters constants;
	Limits *pair = createWaveformPairLimitsFrom(fileName, &constants, &numberOfPairs);
	string matchName;
	sprintf(matchName, "%s/%s", program->outputDirectory, "match.dat");
	FILE *matchFile = safelyOpenForWriting(matchName);
	double step[2], max[2], min[2];
	if (steps->massSet) {
		double mass1[MINMAX] =
			{ pair[0].binary.mass.mass[0][MIN], pair[0].binary.mass.mass[0][MAX] };
		double mass2[MINMAX] =
			{ pair[0].binary.mass.mass[1][MIN], pair[0].binary.mass.mass[1][MAX] };
		double totalMass[MINMAX] = { mass1[MIN] + mass2[MIN], mass1[MAX] + mass2[MAX] };
		double eta[MINMAX] = { 0, 0.25 };
		max[0] = totalMass[MAX];
		min[0] = totalMass[MIN];
		max[1] = 0.25;
		min[1] = 1e-10;
		step[0] = (totalMass[MAX] - totalMass[MIN]) / (double) steps->step[0];
		step[1] = (eta[MAX] - eta[MIN]) / (double) steps->step[1];
	} else if (steps->chiSet) {
		max[0] = max[1] = 1.0;
		min[0] = min[1] = 0.0;
		step[0] = (max[0] - min[0]) / steps->step[0];
		step[1] = (max[1] - min[1]) / steps->step[1];
	}
	size_t current = 0;
	SystemParameter parameter;
	getSysemParametersFromLimits(&pair[0], &constants, copy, &parameter);
	double backup = parameter.samplingFrequency;
	if (numberOfPairs) {
		for (size_t currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			for (double currentOuter = max[0]; currentOuter >= min[0];
				currentOuter -= step[0], current++) {
				for (double currentInner = max[1]; currentInner >= min[1];
					currentInner -= step[1], current++) {
					if (steps->massSet) {
						parameter.system[0].mass.totalMass = parameter.system[1].mass.totalMass =
							currentOuter;
						parameter.system[0].mass.eta = parameter.system[1].mass.eta = currentInner;
						parameter.system[0].mass.mass[0] = parameter.system[1].mass.mass[0] = (1.0
							+ sqrt(1.0 - 4.0 * currentInner)) * currentOuter / 2.0;
						parameter.system[0].mass.mass[1] = parameter.system[1].mass.mass[1] = (1.0
							- sqrt(1.0 - 4.0 * currentInner)) * currentOuter / 2.0;
						if (parameter.system[0].mass.mass[0] < 3.0
							|| parameter.system[0].mass.mass[1] < 3.0) {
							continue;
						}
					} else if (steps->chiSet) {
						parameter.system[0].spin[0].magnitude = parameter.system[1].spin[0]
							.magnitude = currentOuter;
						parameter.system[0].spin[1].magnitude = parameter.system[1].spin[1]
							.magnitude = currentInner;
						convertSpin(&parameter.system[0].spin[0], parameter.system[0].inclination);
						convertSpin(&parameter.system[0].spin[1], parameter.system[0].inclination);
						convertSpin(&parameter.system[1].spin[0], parameter.system[1].inclination);
						convertSpin(&parameter.system[1].spin[1], parameter.system[1].inclination);
					}
					run(program, &parameter, current);
					parameter.samplingFrequency = backup;
					printMassAndSpinsForStatistic(matchFile, &parameter.system[0], parameter.match);
				}
			}
		}
	}
	fclose(matchFile);
}

static void runForWaveformPairs(cstring fileName, bool copy, ProgramParameter *program) {
	ConstantParameters constants;
	size_t numberOfPairs;
	Limits *pair = createWaveformPairLimitsFrom(fileName, &constants, &numberOfPairs);
	string configName;
	string matchName;
	sprintf(matchName, "%s/%s", program->outputDirectory, "match.dat");
	FILE *matchFile = safelyOpenForWriting(matchName);
	string name;
	getFileName(name, fileName);
	sprintf(configName, "%s/data_%s", program->outputDirectory, name);
	FILE *file = safelyOpenForWriting(configName);
	printStartOfConfigFile(file, &constants);
	if (numberOfPairs) {
		for (size_t currentPair = 0; currentPair < numberOfPairs; currentPair++) {
			for (size_t pairMultiply = 0; pairMultiply < pair[2 * currentPair + 1].numberOfRuns;
				pairMultiply++) {
				SystemParameter parameter;
				getSysemParametersFromLimits(&pair[2 * currentPair], &constants, copy, &parameter);
				run(program, &parameter, pairMultiply);
				if (copy) {
					printMassAndSpinsForStatistic(matchFile, &parameter.system[0], parameter.match);
				}
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
	size_t numberOfPairs;
	SystemParameter *pair = createExactWaveformPairFrom(fileName, &numberOfPairs);
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
	initializeRandomGenerator(program.seed);
	strcat(program.outputDirectory, "/");
	string name;
	getFileName(name, parameterFileName);
	strcat(program.outputDirectory, strtok(name, "."));
	makeDir(program.outputDirectory);
	if (option->exact) {
		runForExactWaveformPairs(parameterFileName, &program);
	} else {
		if (option->step.set) {
			runWithStep(parameterFileName, &option->step, option->copy, &program);
		} else {
			runForWaveformPairs(parameterFileName, option->copy, &program);
		}
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
