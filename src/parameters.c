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
	SystemParameter *parameter, ushort blackhole) {
	generateBinarySystemParameters(&parameter->system[blackhole], &limit->binary);
	strcpy(parameter->approximant[blackhole], limit->approximant);
	strcpy(parameter->phase[blackhole], limit->phase);
	strcpy(parameter->spin[blackhole], limit->spin);
	strcpy(parameter->amplitude[blackhole], limit->amplitude);
	parameter->initialFrequency = constants->initialFrequency;
	parameter->samplingFrequency = constants->samplingFrequency;
	parameter->endingFrequency = constants->endingFrequency;
}

void getSysemParametersFromLimits(Limits limit[], ConstantParameters *constants,
	SystemParameter *parameter) {
	for (ushort blackhole = 0; blackhole < NUMBER_OF_BLACKHOLES; blackhole++) {
		getSysemParametersFromLimit(&limit[blackhole], constants, parameter, blackhole);
	}
}

void createFormats(size_t number, Formats *formats) {
	formats->number = number;
	formats->precision = calloc(formats->number, sizeof(ushort));
	formats->width = calloc(formats->number, sizeof(ushort));
	formats->name = calloc(formats->number, sizeof(string));
}

void destroyFormats(Formats *formats) {
	free(formats->precision);
	free(formats->width);
	free(formats->name);
	formats->number = 0;
}

static void printMassLimits(FILE *file, massLimits* mass) {
	fprintf(file, "mass1: %lg %lg\n", mass->mass[0][MIN], mass->mass[0][MAX]);
	fprintf(file, "mass2: %lg %lg\n", mass->mass[1][MIN], mass->mass[1][MAX]);
}

static void printSpinLimits(FILE *file, spinLimits *spin) {
	fprintf(file, "magnitude: %lg %lg\n", spin->magnitude[MIN], spin->magnitude[MAX]);
	fprintf(file, "inclination: %lg %lg\n", spin->inclination[PRECESSING][MIN],
		spin->inclination[PRECESSING][MAX]);
	fprintf(file, "azimuth: %lg %lg\n", spin->azimuth[PRECESSING][MIN],
		spin->azimuth[PRECESSING][MAX]);
}

static void printBinaryLimits(FILE *file, binaryLimits *binary) {
	printMassLimits(file, &binary->mass);
	printSpinLimits(file, &binary->spin[0]);
	printSpinLimits(file, &binary->spin[1]);
	fprintf(file, "incl: %lg %lg\n", binary->inclination[MIN], binary->inclination[MAX]);
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
		fprintf(file, "#system_%d - m_0, m_1, totalMass, eta:     ", i);
		fprintf(file, formatString, param->system[i].mass.mass[0], param->system[i].mass.mass[1],
			param->system[i].mass.totalMass, param->system[i].mass.eta);
	}
	number = 6;
	length = (ushort) (number * format->widthWithSeparator);
	setFormatEnd(formatString, number, format);
	for (ushort i = ZERO; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "#system_%d - c_0, k_0, p_0, c_1, k_1, p_1: ", i);
		fprintf(file, formatString, param->system[i].spin[0].magnitude,
			param->system[i].spin[0].inclination, param->system[i].spin[0].azimuth,
			param->system[i].spin[1].magnitude, param->system[i].spin[1].inclination,
			param->system[i].spin[1].azimuth);
	}
	for (ushort i = ZERO; i < NUMBER_OF_SYSTEMS; i++) {
		fprintf(file, "#system_%d -    approx, phase, spin, ampl: ", i);
		fprintf(file, "%s %s %s %s\n", param->approximant[i], param->phase[i], param->spin[i],
			param->amplitude[i]);
	}
	number = 3;
	length = (ushort) (number * format->widthWithSeparator);
	setFormatEnd(formatString, number, format);
	fprintf(file, "#matches -             minimax, typical, best: ");
	fprintf(file, formatString, match[WORST], match[TYPICAL], match[BEST]);
}
