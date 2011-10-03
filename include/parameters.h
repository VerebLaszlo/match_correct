/**
 * @file parameters.h
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "binary_system.h"
#include "detector.h"

typedef enum ParameterConstants_ {
	TO_PLOT, TO_BACKUP, NUMBER_OF_FORMATS, NUMBER_OF_SYSTEMS = 2, LENGTH_OF_STRING = 100,
} ParameterConstants;

typedef struct {
	double magnitude[MINMAX];
	double inclination[MINMAX];
	double azimuth[MINMAX];
} SpinLimits;

static void printSpinLimits(FILE *file, SpinLimits *spin) {
	fprintf(file, "magnitude: %lg %lg\n", spin->magnitude[MIN], spin->magnitude[MAX]);
	fprintf(file, "inclination: %lg %lg\n", spin->inclination[MIN], spin->inclination[MAX]);
	fprintf(file, "azimuth: %lg %lg\n", spin->azimuth[MIN], spin->azimuth[MAX]);
}

typedef struct {
	double mass[NUMBER_OF_BLACKHOLES][MINMAX];
	SpinLimits spin[NUMBER_OF_BLACKHOLES];
	double inclination[MINMAX];
	double distance[MINMAX];
} SourceLimits;

static void printSourceLimits(FILE *file, SourceLimits *source) {
	fprintf(file, "mass1: %lg %lg\n", source->mass[0][MIN], source->mass[0][MAX]);
	fprintf(file, "mass2: %lg %lg\n", source->mass[1][MIN], source->mass[1][MAX]);
	printSpinLimits(file, &source->spin[0]);
	printSpinLimits(file, &source->spin[1]);
	fprintf(file, "incl: %lg %lg\n", source->inclination[MIN], source->inclination[MAX]);
	fprintf(file, "dist: %lg %lg\n", source->distance[MIN], source->distance[MAX]);
}

typedef struct {
	SourceLimits source;
	char approximant[LENGTH_OF_STRING];
	char phase[LENGTH_OF_STRING];
	char spin[LENGTH_OF_STRING];
	char amplitude[LENGTH_OF_STRING];
	char name[LENGTH_OF_STRING];
} Limits;

static void printLimits(FILE *file, Limits *limit) {
	printSourceLimits(file, &limit->source);
	fprintf(file, "appr: %s\n", limit->approximant);
	fprintf(file, "phase: %s\n", limit->phase);
	fprintf(file, "spin: %s\n", limit->spin);
	fprintf(file, "ampl: %s\n", limit->amplitude);
	fprintf(file, "name: %s\n", limit->name);
}
typedef enum ParameterConstants_ {
	TO_PLOT, TO_BACKUP, NUMBER_OF_FORMATS, NUMBER_OF_SYSTEMS = 2, LENGTH_OF_STRING = 100,
} ParameterConstants;


typedef struct SystemParameter_ {
	BinarySystem system[NUMBER_OF_SYSTEMS];
	DetectorParameters detector[NUMBER_OF_SYSTEMS];
	double coalescencePhase[NUMBER_OF_SYSTEMS];
	double coalescenceTime[NUMBER_OF_SYSTEMS];
	double samplingFrequency;
	double samplingTime;
	double initialFrequency;
	double endingFrequency;
	char name[NUMBER_OF_SYSTEMS][LENGTH_OF_STRING];
	char approximant[NUMBER_OF_SYSTEMS][LENGTH_OF_STRING];
	char phase[NUMBER_OF_SYSTEMS][LENGTH_OF_STRING];
	char spin[NUMBER_OF_SYSTEMS][LENGTH_OF_STRING];
	char amplitude[NUMBER_OF_SYSTEMS][LENGTH_OF_STRING];
} SystemParameter;

typedef struct ProgramParameter_ {
	char outputDirectory[FILENAME_MAX];
	ulong numberOfRuns;
	ushort precision[NUMBER_OF_FORMATS];
	ushort width[NUMBER_OF_FORMATS];
} ProgramParameter;

void readExactParameters(FILE *file, SystemParameter *params);

void readSystemParameters(FILE *file, SystemParameter *params);

void readProgramParameters(FILE *file, ProgramParameter *params);

void printProgramParameters(FILE *file, ProgramParameter *params);

void printSystemParameters(FILE *file, SystemParameter *params, OutputFormat *format);

#endif /* PARAMETERS_H_ */
