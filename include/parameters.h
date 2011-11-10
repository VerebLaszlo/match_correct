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

typedef struct {
	char parameterFile[FILENAME_MAX];
	char programFile[FILENAME_MAX];
	bool plot;
	bool calculateMatch;
	bool testing;
	bool exact;
	bool copy;
} Options;

typedef enum ParameterConstants_ {
	NUMBER_OF_SYSTEMS = 2, LENGTH_OF_STRING = 100,
} ParameterConstants;

typedef struct {
	size_t maxNumberOfRuns;
	double initialFrequency;
	double samplingFrequency;
	double endingFrequency;
} ConstantParameters;

typedef struct {
	size_t numberOfRuns;
	binaryLimits binary;
	char approximant[LENGTH_OF_STRING];
	char phase[LENGTH_OF_STRING];
	char spin[LENGTH_OF_STRING];
	char amplitude[LENGTH_OF_STRING];
	char name[LENGTH_OF_STRING];
} Limits;

typedef enum {
	WORST, TYPICAL, BEST, NUMBER_OF_MATCHES,
} MATCHES;

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
	double match[NUMBER_OF_MATCHES];
	size_t numberOfRuns;
} SystemParameter;

void getSysemParametersFromLimit(Limits *limit, ConstantParameters *constants,
	SystemParameter *parameter, ushort blackhole);

void getSysemParametersFromLimits(Limits *limit, ConstantParameters *constants, bool copy,
	SystemParameter *parameter);

typedef struct {
	size_t number;
	ushort *precision;
	ushort *width;
	string *name;
} Formats;

void createFormats(size_t number, Formats *formats);

void destroyFormats(Formats *formats);

typedef struct ProgramParameter_ {
	char outputDirectory[FILENAME_MAX];
	Formats format;
	ulong numberOfRuns;
	int seed;
	bool plot;
	bool calculateMatches;
} ProgramParameter;

void printLimits(FILE *file, Limits *limit);

void printProgramParameters(FILE *file, ProgramParameter *params);

void printStartOfConfigFile(FILE *file, ConstantParameters *constant);

void printMiddleOfConfigFile(FILE *file);

void printEndOfConfigFile(FILE *file);

void printWaveformPairsToConfigFile(FILE *file, SystemParameter *param, OutputFormat *format);

void printSystemParameters(FILE *file, SystemParameter *params, OutputFormat *format);

void printMassAndSpinsForStatistic(FILE *file, BinarySystem *param, double match[]);

void printParametersForSignalPlotting(FILE *file, SystemParameter *param, double match[]);

#endif /* PARAMETERS_H_ */
