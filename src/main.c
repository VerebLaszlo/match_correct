/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "generator_lal.h"
#include "util_IO.h"

static void printConfig(void) {
	FILE*file = safelyOpenForWriting("base.conf");
	fputs("output = \"out/test\"\n"
			"units {	angle = \"degree\" mass = \"solar\" distance = \"Mpc\" }\n"
			"boundaryFrequency = {30.0, 500.0}\n"
			"samplingFrequency = 10240.0\n"
			"\n"
			"wave default {\n"
			"	binary {\n"
			"		mass = {3.0, 30.0}\n"
			"		spin {\n"
			"			magnitude = {1.0, 1.0}\n"
			"			inclination = {90.0, 180.0}\n"
			"			azimuth = {0.0, 60.0}\n"
			"			coorSystem = \"precessing\"\n"
			"		}\n"
			"		inclination = 10.0\n"
			"		distance = 1.0\n"
			"	}\n"
			"	method {\n"
			"		spin = \"ALL\" phase = 4 amplitude = 0\n"
			"	}\n"
			"}\n"
			"\n"
			"pair qm {\n"
			"	wave {method {spin = \"SO\"}}\n"
			"	wave {method {spin = \"SOQM\"}}\n"
			"}\n"
			"\n"
			"step default {\n"
			"	wave {\n"
			"		binary {\n"
			"			mass = {3.0, 3.0}\n"
			"			spin {\n"
			"				magnitude = {0.0, 0.0}\n"
			"				inclination = {0.0, 0.0}\n"
			"				azimuth = {0.0, 0.0}\n"
			"				coorSystem = \"precessing\"\n"
			"			}\n"
			"			inclination = 0.0\n"
			"			distance = 1.0\n"
			"		}\n"
			"	}\n"
			"	wave {\n"
			"		binary {\n"
			"			mass = {30.0, 30.0}\n"
			"			spin {\n"
			"				magnitude = {1.0, 1.0}\n"
			"				inclination = {180.0, 180.0}\n"
			"				azimuth = {360.0, 360.0}\n"
			"				coorSystem = \"precessing\"\n"
			"			}\n"
			"			inclination = 180.0\n"
			"			distance = 1.0\n"
			"		}\n"
			"	}\n"
			"	diff = {1, 1}\n"
			"	gen = {false, false, true}\n"
			"}\n"
			"\n"
			"step qm {\n"
			"	wave {method {spin = \"SO\"}}\n"
			"	wave {method {spin = \"ALL\"}}\n"
			"}\n", file);
	fclose(file);
}

static void print(Variable *variable, Wave parameter[2], Analysed *analysed, char *name, double samplingTime,
        string outputDir) {
	string path;
	FILE *file;
	sprintf(path, "%s/%s_spin.txt", outputDir, name);
	file = safelyOpenForWriting(path);
	printSpins(file, variable, parameter, analysed, samplingTime);
	fclose(file);
	sprintf(path, "%s/%s_system.txt", outputDir, name);
	file = safelyOpenForWriting(path);
	printSystem(file, variable, parameter, analysed, samplingTime);
	fclose(file);
	sprintf(path, "%s/%s_wave.txt", outputDir, name);
	file = safelyOpenForWriting(path);
	printOutput(file, variable, parameter, analysed, samplingTime);
	fclose(file);
}

static int generateWaveforms(char *input, Parameter *parameter, string outputDir) {
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
			        parameter->samplingTime, outputDir);
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

string fileName;

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
	switch (variable) {
	case MASS:
		strcpy(fileName, "mass");
		break;
	case MAGNITUDE:
		strcpy(fileName, "magn");
		break;
	case INCLINATION:
		strcpy(fileName, "incl");
		break;
	case AZIMUTH:
		strcpy(fileName, "azim");
		break;
	default:
		break;
	}
}

static int printHeader(FILE *file, Wave parameter[], Value variable) {
	if (variable != MASS) {
		double M = parameter->binary.mass[0] + parameter->binary.mass[1];
		double eta = parameter->binary.mass[0] * parameter->binary.mass[1] / square(M);
		fprintf(file, "#mass  [m1,m2,M,eta] %11.5g %11.5g %11.5g %11.5g\n", parameter->binary.mass[0],
		        parameter->binary.mass[1], M, eta);
	} else {
		fprintf(file, "#mass  [m1,m2,M,eta] %11s %11s %11s %11s\n", "X", "X", "X", "X");
	}
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		if (variable == MAGNITUDE) {
			fprintf(file, "#spin%d [mag,inc,azi] %11s %11.5g %11.5g\n", blackhole, "X",
			        degreeFromRadian(parameter->binary.spin.inclination[blackhole]),
			        degreeFromRadian(parameter->binary.spin.azimuth[blackhole]));
		}
		if (variable == INCLINATION) {
			fprintf(file, "#spin%d [mag,inc,azi] %11.5g %11s %11.5g\n", blackhole,
			        parameter->binary.spin.magnitude[blackhole], "X",
			        degreeFromRadian(parameter->binary.spin.azimuth[blackhole]));
		}
		if (variable == AZIMUTH) {
			fprintf(file, "#spin%d [mag,inc,azi] %11.5g %11.5g %11s\n", blackhole,
			        parameter->binary.spin.magnitude[blackhole],
			        degreeFromRadian(parameter->binary.spin.inclination[blackhole]), "X");
		}
	}
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		fprintf(file, "#method[int, pn,amp] %11s %11d %11d\n", parameter[wave].method.spin,
		        parameter[wave].method.phase, parameter[wave].method.amplitude);
	}
	return (SUCCESS);
}

static int generateStatistic(char *input, Parameter *parameter, string outputDir) {
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
	size_t minIndex, maxIndex;
	Wave pair[NUMBER_OF_WAVE];
	Variable *generated;
	FILE *file;
	for (size_t current = FIRST; current < parameter->step->length; current++) {
		memcpy(pair, &parameter->step->wave[2 * current], 2 * sizeof(Wave));
		for (int variable = MASS; variable < NUMBER_OF_VARIABLE; variable++) {
			if (!parameter->gen[variable]) {
				continue;
			}
			double value[THIRD] = { bounds[MIN][variable][FIRST], bounds[MIN][variable][SECOND] };
			double diff[THIRD] = { (bounds[MAX][variable][FIRST] - bounds[MIN][variable][FIRST])
			        / (parameter->numberOfStep[FIRST] - 1), (bounds[MAX][variable][SECOND]
			        - bounds[MIN][variable][SECOND]) / (parameter->numberOfStep[SECOND] - 1) };
			set(variable, pair, value);
			string path;
			sprintf(path, "%s/%s.data", outputDir, fileName);
			printf("%s\n", path);
			file = safelyOpenForWriting(path);
			printHeader(file, pair, variable);
			if (variable == MASS) {
				fprintf(file, "#%10s %11s  ", "totalMass", "eta");
			} else {
				fprintf(file, "#");
			}
			fprintf(file, "%9s1 %10s2 %11s %11s %11s %11s %11s\n", fileName, fileName, "worst", "typical", "best",
			        "relPeriod", "relLength");
			while (value[FIRST] < bounds[MAX][variable][FIRST] + diff[FIRST]) {
				value[SECOND] = bounds[MIN][variable][SECOND];
				set(variable, pair, value);
				while (value[SECOND] < bounds[MAX][variable][SECOND] + diff[SECOND]) {
					set(variable, pair, value);
					generated = generateWaveformPair(pair, parameter->initialFrequency, parameter->samplingTime);
					initMatch(generated->wave);
					generatePSD(parameter->initialFrequency, parameter->samplingFrequency);
					indexFromFrequency(parameter->initialFrequency, parameter->endingFrequency,
					        parameter->samplingFrequency / generated->size, &minIndex, &maxIndex);
					Analysed analysed;
					calcMatches(minIndex, maxIndex, &analysed);
					countPeriods(parameter->samplingTime, &analysed);
					if (variable == MASS) {
						double totalMass = value[FIRST] + value[SECOND];
						double eta = value[FIRST] * value[SECOND] / square(totalMass);
						fprintf(file, "%11.5g %11.5g ", totalMass, eta);
					}
					fprintf(file, "%11.5g %11.5g %11.5g %11.5g %11.5g %11.5g %11.5g\n", value[FIRST], value[SECOND],
					        analysed.match[WORST], analysed.match[TYPICAL], analysed.match[BEST],
					        analysed.relativePeriod, analysed.relativeLength);
					cleanMatch();
					destroyWaveform(&generated->wave);
					destroyOutput(&generated);
					value[SECOND] += diff[SECOND];
				}
				value[FIRST] += diff[FIRST];
			}
			fclose(file);
		}
	}
	return (failure);
}

static int initDirectory(string output, string input) {
	char *fileName = strrchr(input, '/');
	if (fileName) {
		fileName++;
	} else {
		fileName = input;
	}
	int i;
	i = strcspn(fileName, ".");
	string dir;
	memset(dir, 0, sizeof(string));
	strncpy(dir, fileName, i);
	sprintf(output, "%s/%s", output, dir);
	mkdir(output, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return (SUCCESS);
}

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	printConfig();
	char *input = argc > 1 ? argv[1] : "test.conf";
	Parameter parameter;
	memset(&parameter, 0, sizeof(Parameter));
	string outputDir;
	initParser(input, &parameter, outputDir);
	initDirectory(outputDir, input);
	printf("%s\n", outputDir);
	int failure = SUCCESS;
	if (parameter.exactTrue) {
		failure = generateWaveforms(input, &parameter, outputDir);
	}
	if (parameter.stepTrue) {
		failure |= generateStatistic(input, &parameter, outputDir);
	}
	cleanParameter(&parameter);
	if (!failure) {
		puts("OK!");
	} else {
		puts("Error!");
	}
}
