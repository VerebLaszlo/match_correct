/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include <stdlib.h>
#include <string.h>
#include "generator_lal.h"
#include "util_IO.h"

void print(Variable *variable, Wave parameter[2], Analysed *analysed, char *name, double samplingTime) {
	string path;
	FILE *file;
	sprintf(path, "out/%s_spin.txt", name);
	file = safelyOpenForWriting(path);
	printSpins(file, variable, parameter, analysed, samplingTime);
	fclose(file);
	sprintf(path, "out/%s_system.txt", name);
	file = safelyOpenForWriting(path);
	printSystem(file, variable, parameter, analysed, samplingTime);
	fclose(file);
	sprintf(path, "out/%s_wave.txt", name);
	file = safelyOpenForWriting(path);
	printOutput(file, variable, parameter, analysed, samplingTime);
	fclose(file);
}

static int generateWaveforms(char *input, Parameter *parameter) {
	int failure = SUCCESS;
	failure &= parseWaves(input, parameter);
	size_t minIndex, maxIndex;
	if (!failure) {
		Variable *variable;
		for (size_t index = 0; index < parameter->exact->length; index++) {
			variable = generateWaveformPair(&parameter->exact->wave[2 * index], parameter->initialFrequency,
			        parameter->samplingTime);
			initMatch(variable->wave);
			generatePSD(parameter->initialFrequency, parameter->samplingFrequency);
			indexFromFrequency(parameter->initialFrequency, parameter->endingFrequency,
			        parameter->samplingFrequency / variable->size, &minIndex, &maxIndex);
			Analysed analysed;
			calcMatches(minIndex, maxIndex, &analysed);
			countPeriods(parameter->samplingTime, &analysed);
			printf("w:%g t:%g b:%g\n%d %d %g%%\n%g %g %g%%\n", analysed.match[WORST], analysed.match[TYPICAL],
			        analysed.match[BEST], analysed.period[FIRST_WAVE], analysed.period[SECOND_WAVE],
			        analysed.relativePeriod * 100.0, analysed.length[FIRST_WAVE], analysed.length[SECOND_WAVE],
			        analysed.relativeLength * 100.0);
			print(variable, &parameter->exact->wave[2 * index], &analysed, parameter->exact->name[index],
			        parameter->samplingTime);
			cleanMatch();
			destroyWaveform(&variable->wave);
			destroyOutput(&variable);
		}
	}
	return (failure);
}

static int generateStatistic(char *input, Parameter *parameter) {
	int failure = SUCCESS;
	failure &= parseStep(input, parameter);
	if (!failure) {
		printParameter(stdout, parameter, 0, 0);
	}
	return (failure);

}

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	char *input = argc > 1 ? argv[1] : "test.conf";
	Parameter parameter;
	memset(&parameter, 0, sizeof(Parameter));
	initParser(input, &parameter);
	int failure = SUCCESS;
	failure = generateWaveforms(input, &parameter);
	failure |= generateStatistic(input, &parameter);
	cleanParameter(&parameter);
	if (!failure) {
		puts("OK!");
	} else {
		puts("Error!");
	}
}
