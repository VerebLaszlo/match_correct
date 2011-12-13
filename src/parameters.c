/**
 * @file parameters.c
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "parameters.h"

void getSysemParametersFromLimit(Limits *limit, ConstantParameters *constants,
	SystemParameter *parameter, ushort systems) {
	generateBinarySystemParameters(&parameter->system[systems], &limit->binary);
	strcpy(parameter->approximant[systems], limit->approximant);
	strcpy(parameter->phase[systems], limit->phase);
	strcpy(parameter->spin[systems], limit->spin);
	strcpy(parameter->amplitude[systems], limit->amplitude);
	strcpy(parameter->name[systems], limit->name);
	parameter->numberOfRuns = limit->numberOfRuns;
	parameter->initialFrequency = constants->initialFrequency;
	parameter->endingFrequency = constants->endingFrequency;
	parameter->samplingFrequency = constants->samplingFrequency;
	parameter->samplingTime = 1.0 / parameter->samplingFrequency;
}

void getSysemParametersFromLimits(Limits limit[], ConstantParameters *constants, bool copy,
	SystemParameter *parameter) {
	for (ushort systems = 0; systems < NUMBER_OF_SYSTEMS; systems++) {
		getSysemParametersFromLimit(&limit[systems], constants, parameter, systems);
	}
	if (copy) {
		memcpy(&parameter->system[1], &parameter->system[0], sizeof(BinarySystem));
	}
}

void createFormats(size_t number, Formats *formats) {
	formats->number = number;
	formats->precision = secureCalloc(formats->number, sizeof(ushort));
	formats->width = secureCalloc(formats->number, sizeof(ushort));
	formats->name = secureCalloc(formats->number, sizeof(string));
}

void destroyFormats(Formats *formats) {
	secureFree(formats->precision);
	secureFree(formats->width);
	secureFree(formats->name);
	formats->number = 0;
}

static void printMassLimits(FILE *file, massLimits* mass) {
	fprintf(file, "mass1: %lg %lg\n", mass->mass[0][MIN], mass->mass[0][MAX]);
	fprintf(file, "mass2: %lg %lg\n", mass->mass[1][MIN], mass->mass[1][MAX]);
}

static void printSpinLimits(FILE *file, spinLimits *spin) {
	fprintf(file, "magnitude: %lg %lg\n", spin->magnitude[MIN], spin->magnitude[MAX]);
	fprintf(file, "inclination: %lg %lg\n", degreeFromRadian(spin->inclination[PRECESSING][MIN]),
		degreeFromRadian(spin->inclination[PRECESSING][MAX]));
	fprintf(file, "azimuth: %lg %lg\n", degreeFromRadian(spin->azimuth[PRECESSING][MIN]),
		degreeFromRadian(spin->azimuth[PRECESSING][MAX]));
}

static void printBinaryLimits(FILE *file, binaryLimits *binary) {
	printMassLimits(file, &binary->mass);
	printSpinLimits(file, &binary->spin[0]);
	printSpinLimits(file, &binary->spin[1]);
	fprintf(file, "incl: %lg %lg\n", degreeFromRadian(binary->inclination[MIN]),
		degreeFromRadian(binary->inclination[MAX]));
	fprintf(file, "dist: %lg %lg\n", binary->distance[MIN], binary->distance[MAX]);
}

void printLimits(FILE *file, Limits *limit) {
	printBinaryLimits(file, &limit->binary);
	fprintf(file, "appr: %s\n", limit->approximant);
	fprintf(file, "phase: %s\n", limit->phase);
	fprintf(file, "spin: %s\n", limit->spin);
	fprintf(file, "ampl: %s\n", limit->amplitude);
	fprintf(file, "name: %s\n", limit->name);
}

void printProgramParameters(FILE *file, ProgramParameter *params) {
	fprintf(file, "%10s %10ld\n", "numOfRuns", params->numberOfRuns);
	fprintf(file, "%10s %10hd\n", "prec", params->format.precision[SIGNAL_DATA]);
	fprintf(file, "%10s %10d\n", "width", params->format.width[SIGNAL_DATA]);
	fprintf(file, "%10s %10hd\n", "precPlot", params->format.precision[SIGNAL_DATA]);
	fprintf(file, "%10s %10d\n", "widthPlot", params->format.width[SIGNAL_DATA]);
	fprintf(file, "%10s %10s\n", "folder", params->outputDirectory);
}

static void printWaveParametersToConfigFile(FILE *file, SystemParameter *param,
	ushort currentSystem, OutputFormat *format) {
	printBinarySystemToConfig(file, &param->system[currentSystem], format);
	fputs("\t\t\tgeneration = {", file);
	fprintf(file, " approximant = \"%s\";", param->approximant[currentSystem]);
	fprintf(file, " phase = \"%s\"; ", param->phase[currentSystem]);
	fprintf(file, " spin = \"%s\"; ", param->spin[currentSystem]);
	fprintf(file, " amplitude = \"%s\"; ", param->amplitude[currentSystem]);
	fputs("};\n", file);
}

static void printConstantsToConfigFile(FILE *file, ConstantParameters *constant) {
	fputs("units = { angle = \"degree\"; mass = \"solar\"; };\n", file);
	fprintf(file, "boundaryFrequency = [%lg, %lg]; samplingFrequency = %lg;\n\n",
		constant->initialFrequency, constant->endingFrequency, constant->samplingFrequency);
}

void printStartOfConfigFile(FILE *file, ConstantParameters *constant) {
	printConstantsToConfigFile(file, constant);
	fputs("pairs = (\n", file);
	fputs("\t(\n", file);
}

void printMiddleOfConfigFile(FILE *file) {
	fputs("\t),(\n", file);
}

void printEndOfConfigFile(FILE *file) {
	fputs("\t)\n", file);
	fputs(");", file);
}

void printWaveformPairsToConfigFile(FILE *file, SystemParameter *param, OutputFormat *format) {
	fputs("\t\t{\n", file);
	printWaveParametersToConfigFile(file, param, 0, format);
	fputs("\t\t}, {\n", file);
	printWaveParametersToConfigFile(file, param, 1, format);
	fputs("\t\t},\n", file);
	fprintf(file, "\t\t\"%s\"\n", param->name[1]);
}

void printSystemParameters(FILE *file, SystemParameter *params, OutputFormat *format) {
	fprintf(file, "%10s %10.4lg\n", "freq_I", params->initialFrequency);
	fprintf(file, "%10s %10.4lg\n", "freq_E", params->endingFrequency);
	fprintf(file, "%10s %10.4lg\n", "freq_S", params->samplingFrequency);
	fprintf(file, "%10s %10.4lg\n", "time_S", params->samplingTime);
	for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "%10s %10s\n", "name", params->name[i]);
		fprintf(file, "%10s %10s\n", "approx", params->approximant[i]);
		fprintf(file, "%10s %10s\n", "phase", params->phase[i]);
		fprintf(file, "%10s %10s\n", "spin", params->spin[i]);
		fprintf(file, "%10s %10s\n", "amp", params->amplitude[0]);
		printBinarySystemParameters(file, &params->system[i], format);
	}
}

void printParametersForSignalPlotting(FILE *file, SystemParameter *param, double match[]) {
	OutputFormat *format = &outputFormat[SIGNAL_DATA];
	ushort number = 4;
	ushort length = (ushort) (number * format->widthWithSeparator);
	char formatString[length];
	setFormatEnd(formatString, number, format);
	for (ushort i = ZERO; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "#system_%d -     m_0, m_1, totalMass, eta: ", i);
		fprintf(file, formatString, param->system[i].mass.mass[0], param->system[i].mass.mass[1],
			param->system[i].mass.totalMass, param->system[i].mass.eta);
	}
	number = 6;
	length = (ushort) (number * format->widthWithSeparator);
	setFormatEnd(formatString, number, format);
	for (ushort i = ZERO; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "#system_%d - c_0, k_0, p_0, c_1, k_1, p_1: ", i);
		fprintf(file, formatString, param->system[i].spin[0].magnitude,
			degreeFromRadian(param->system[i].spin[0].inclination[PRECESSING]),
			degreeFromRadian(param->system[i].spin[0].azimuth[PRECESSING]),
			param->system[i].spin[1].magnitude,
			degreeFromRadian(param->system[i].spin[1].inclination[PRECESSING]),
			degreeFromRadian(param->system[i].spin[1].azimuth[PRECESSING]));
	}
	for (ushort i = ZERO; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "#system_%d -    approx, phase, spin, ampl: ", i);
		fprintf(file, "%s %s %s %s\n", param->approximant[i], param->phase[i], param->spin[i],
			param->amplitude[i]);
	}
	number = 3;
	length = (ushort) (number * format->widthWithSeparator);
	setFormatEnd(formatString, number, format);
	fprintf(file, "#matches  -       minimax, typical, best: ");
	fprintf(file, formatString, match[WORST], match[TYPICAL], match[BEST]);
	number = 2;
	length = (ushort) (number * format->widthWithSeparator);
	setFormatEnd(formatString, number, format);
	fprintf(file, "#periods  -                             : %d %d\n", param->periods[0], param->periods[1]);
}

void printMassAndSpinsForStatistic(FILE *file, BinarySystem *param, double match[],
	size_t periods[]) {
	OutputFormat *format = &outputFormat[SIGNAL_TO_PLOT];
	ushort number = 3 + 2 + 2 * 3 + 2;
	ushort length = number * format->widthWithSeparator;
	char formatString[length];
	setFormatEnd(formatString, number, format);
	fprintf(file, formatString, match[WORST], match[TYPICAL], match[BEST], param->mass.totalMass,
		param->mass.eta, param->spin[0].magnitude, param->spin[1].magnitude,
		param->spin[0].inclination[PRECESSING], param->spin[1].inclination[PRECESSING],
		param->spin[0].azimuth[PRECESSING], param->spin[1].azimuth[PRECESSING], periods[0],
		periods[1]);
}
