/**
 * @file parser.c
 * @author vereb
 * @date Sep 28, 2011
 * @brief 
 */

#include <libconfig.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "util_math.h"

typedef enum {
	BOUNDARY_FREQUENCY,
	SAMPLING_FREQUENCY,
	DEFAULT,
	SOURCE,
	BINARY,
	MASSES,
	SPINS,
	MAGNITUDE,
	INCLINATION,
	AZIMUTH,
	DISTANCE,
	DETECTOR,
	GENERATION,
	APPROXIMANT,
	PHASE,
	SPIN,
	AMPLITUDE,
	NAME,
	PAIRS,
	SIGNAL,
	TEMPLATES,
	NUMBER_OF_OPTIONS,
} OptionCode;

char const * optionName[] = { "boundaryFrequency", "samplingFrequency", "default", "source",
								"binary", "masses", "spins", "magnitude", "inclination", "azimuth",
								"distance", "detector", "generation", "approximant", "phase",
								"spin", "amplitude", "name", "pairs", "signal", "templates", };

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

static ushort neededElementNumber(ushort number, config_setting_t *elements) {
	ushort count = (ushort) config_setting_length(elements);
	if (count != number) {
		printf("%d\n", count);
		exit(EXIT_FAILURE);
	}
	return count;
}

static void getLimits(config_setting_t *limits, double limit[]) {
	int count = neededElementNumber(MINMAX, limits);
	for (ushort i = 0; i < count; i++) {
		limit[i] = config_setting_get_float_elem(limits, i);
	}
}

static void getMasses(config_setting_t *binary, double defaults[NUMBER_OF_BLACKHOLES][MINMAX],
	double limit[NUMBER_OF_BLACKHOLES][MINMAX]) {
	config_setting_t *masses = config_setting_get_member(binary, optionName[MASSES]);
	int count = neededElementNumber(NUMBER_OF_BLACKHOLES, masses);
	for (ushort i = 0; i < count; i++) {
		config_setting_t *mass = config_setting_get_elem(masses, i);
		getLimits(mass, limit[i]);
	}
}

static void getSpin(config_setting_t *spin, SpinLimits *defaults, SpinLimits *limit) {
	config_setting_t *current;
	current = config_setting_get_member(spin, optionName[MAGNITUDE]);
	getLimits(current, limit->magnitude);
	current = config_setting_get_member(spin, optionName[INCLINATION]);
	getLimits(current, limit->inclination);
	current = config_setting_get_member(spin, optionName[AZIMUTH]);
	getLimits(current, limit->azimuth);
}

static void getSpins(config_setting_t *binary, SourceLimits *defaults, SourceLimits *limit) {
	config_setting_t *spins = config_setting_get_member(binary, optionName[SPINS]);
	int count = config_setting_length(spins);
	for (ushort i = 0; i < count; i++) {
		config_setting_t *spin = config_setting_get_elem(spins, i);
		getSpin(spin, &defaults->spin[i], &limit->spin[i]);
	}
}

static void getSourceParameters(config_setting_t *waveform, SourceLimits *defaults,
	SourceLimits *limit) {
	config_setting_t *source = config_setting_get_member(waveform, optionName[SOURCE]);
	getMasses(source, defaults->mass, limit->mass);
	getSpins(source, defaults, limit);
	config_setting_t *current;
	current = config_setting_get_member(source, optionName[INCLINATION]);
	getLimits(current, limit->inclination);
	current = config_setting_get_member(source, optionName[DISTANCE]);
	getLimits(current, limit->distance);
}

typedef const char *cstring;

static void getGenerationParameters(config_setting_t *waveform, Limits *defaults, Limits *limit) {
	config_setting_t *generation = config_setting_get_member(waveform, optionName[GENERATION]);
	cstring approximant, phase, spin, amplitude;
	config_setting_lookup_string(generation, optionName[APPROXIMANT], &approximant);
	strcpy(limit->approximant, approximant);
	config_setting_lookup_string(generation, optionName[PHASE], &phase);
	strcpy(limit->phase, phase);
	config_setting_lookup_string(generation, optionName[SPIN], &spin);
	strcpy(limit->spin, spin);
	config_setting_lookup_string(generation, optionName[AMPLITUDE], &amplitude);
	strcpy(limit->amplitude, amplitude);
}

static void getWaveformParameters(config_setting_t *waveform, Limits *defaults, Limits *limit) {
	getSourceParameters(waveform, &defaults->source, &limit->source);
	getGenerationParameters(waveform, defaults, limit);
	cstring name;
	config_setting_lookup_string(waveform, optionName[NAME], &name);
	strcpy(limit->name, name);
}

static void getWavePairParameters(config_setting_t *pair, Limits *defaults, Limits limit[]) {
	config_setting_t *current;
	ushort count = (ushort) neededElementNumber(2, pair);
	for (ushort i = 0; i < count; i++) {
		current = config_setting_get_elem(pair, i);
		getWaveformParameters(current, defaults, &limit[i]);
	}
}

void testParser(SystemParameter *parameter) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, "parser.conf")) {
		fprintf(stderr, "%d - %s\n", config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	config_setting_t *defaultWave = config_lookup(&cfg, optionName[DEFAULT]);
	Limits limit;
	getWaveformParameters(defaultWave, NULL, &limit);
	printLimits(stdout, &limit);
	// pairs
	Limits *pairsLimit = NULL;
	config_setting_t *current;
	config_setting_t *pairs = config_lookup(&cfg, optionName[PAIRS]);
	size_t numberOfPairs = (size_t) config_setting_length(pairs);
	pairsLimit = calloc(2 * numberOfPairs, sizeof(Limits));
	for (size_t i = 0; i < numberOfPairs; i++) {
		current = config_setting_get_elem(pairs, i);
		getWavePairParameters(current, &limit, &pairsLimit[2 * i]);
	}
	// signal + templates
	Limits signalLimit;
	config_setting_t *signal = config_lookup(&cfg, optionName[SIGNAL]);
	getWaveformParameters(signal, &limit, &signalLimit);
	Limits *templatesLimit = NULL;
	config_setting_t *templates = config_lookup(&cfg, optionName[TEMPLATES]);
	size_t numberOfTemplates = (size_t) config_setting_length(templates);
	templatesLimit = calloc(numberOfTemplates, sizeof(Limits));
	for (size_t i = 0; i < numberOfTemplates; i++) {
		current = config_setting_get_elem(templates, i);
		getWaveformParameters(current, &limit, &templatesLimit[i]);
	}
	config_destroy(&cfg);
}
