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
		Variable * variable;
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

typedef enum {
	MASS, MAGNITUDE, INCLINATION, AZIMUTH, NUMBER_OF_VARIABLE,
} Value;

static void set(Value variable, Wave *pair, double *value) {
	for (int current = FIRST; current < THIRD; current++) {
		switch (variable) {
		case MASS:
			pair[FIRST].binary.mass[current] = value[current];
			pair[SECOND].binary.mass[current] = value[current];
			break;
		case MAGNITUDE:
			pair[FIRST].binary.spin.magnitude[current] = value[current];
			pair[SECOND].binary.spin.magnitude[current] = value[current];
			break;
		case INCLINATION:
			pair[FIRST].binary.spin.inclination[current] = value[current];
			pair[SECOND].binary.spin.inclination[current] = value[current];
			break;
		case AZIMUTH:
			pair[FIRST].binary.spin.azimuth[current] = value[current];
			pair[SECOND].binary.spin.azimuth[current] = value[current];
			break;
		default:
			break;
		}
	}
}

static int generateStatistic(char *input, Parameter *parameter) {
	int failure = SUCCESS;
	failure &= parseStep(input, parameter);
	double bounds[MINMAX][NUMBER_OF_VARIABLE][BH];
	for (int boundary = MIN; boundary < MINMAX; boundary++) {
		bounds[boundary][MASS][FIRST] = parameter->boundary[boundary].binary.mass[FIRST];
		bounds[boundary][MASS][SECOND] = parameter->boundary[boundary].binary.mass[SECOND];
		bounds[boundary][MAGNITUDE][FIRST] = parameter->boundary[boundary].binary.spin.magnitude[FIRST];
		bounds[boundary][MAGNITUDE][SECOND] = parameter->boundary[boundary].binary.spin.magnitude[SECOND];
		bounds[boundary][INCLINATION][FIRST] = parameter->boundary[boundary].binary.spin.inclination[FIRST];
		bounds[boundary][INCLINATION][SECOND] = parameter->boundary[boundary].binary.spin.inclination[SECOND];
		bounds[boundary][AZIMUTH][FIRST] = parameter->boundary[boundary].binary.spin.azimuth[FIRST];
		bounds[boundary][AZIMUTH][SECOND] = parameter->boundary[boundary].binary.spin.azimuth[SECOND];
	}
	Wave pair[NUMBER_OF_WAVE];
	Variable *generated;
	for (size_t current = FIRST; current < parameter->step->length; current++) {
		memcpy(pair, &parameter->step->wave[2 * current], 2 * sizeof(Wave));
		for (int variable = MASS; variable < NUMBER_OF_VARIABLE; variable++) {
			double value[THIRD] = { bounds[MIN][variable][FIRST], bounds[MIN][variable][SECOND] };
			double diff[THIRD] = { (bounds[MAX][variable][FIRST] - bounds[MIN][variable][FIRST])
			        / (parameter->numberOfStep[FIRST] - 1), (bounds[MAX][variable][SECOND]
			        - bounds[MIN][variable][SECOND]) / (parameter->numberOfStep[SECOND] - 1) };
			set(variable, pair, value);
			while (value[FIRST] < bounds[MAX][variable][FIRST] + diff[FIRST]) {
				value[SECOND] = bounds[MIN][variable][SECOND];
				set(variable, pair, value);
				while (value[SECOND] < bounds[MAX][variable][SECOND] + diff[SECOND]) {
					generated = generateWaveformPair(&parameter->step->wave[2 * current], parameter->initialFrequency,
					        parameter->samplingTime);
					// match
					destroyWaveform(&generated->wave);
					destroyOutput(&generated);
					value[SECOND] += diff[SECOND];
				}
				value[FIRST] += diff[FIRST];
			}
		}
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
