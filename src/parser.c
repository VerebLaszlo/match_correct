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
	MASS1,
	MASS2,
	SPIN1,
	SPIN2,
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
								"binary", "mass1", "mass2", "spin1", "spin2", "magnitude",
								"inclination", "azimuth", "distance", "detector", "generation",
								"approximant", "phase", "spin", "amplitude", "name", "pairs",
								"signal", "templates", };

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

static void getMass(ushort blackhole, config_setting_t *source, double defaults[MINMAX],
	double limit[MINMAX]) {
	config_setting_t *mass = config_setting_get_member(source, optionName[MASS1 + blackhole]);
	if (mass) {
		getLimits(mass, limit);
	} else {
		memcpy(limit, defaults, 2*sizeof(defaults[MINMAX]));
	}
}

static void getMasses(config_setting_t *source, double defaults[NUMBER_OF_BLACKHOLES][MINMAX],
	double limit[NUMBER_OF_BLACKHOLES][MINMAX]) {
	for (ushort blackholes = 0; blackholes < NUMBER_OF_BLACKHOLES; blackholes++) {
		getMass(blackholes, source, defaults[blackholes], limit[blackholes]);
	}
}

static void getSpin(config_setting_t *spin, SpinLimits *defaults, SpinLimits *limit) {
	config_setting_t *current;
	current = config_setting_get_member(spin, optionName[MAGNITUDE]);
	if (current) {
		getLimits(current, limit->magnitude);
	} else {
		memcpy(limit->magnitude, defaults->magnitude, MINMAX * sizeof(double));
	}
	current = config_setting_get_member(spin, optionName[INCLINATION]);
	if (current) {
		getLimits(current, limit->inclination);
	} else {
		memcpy(limit->inclination, defaults->inclination, MINMAX * sizeof(double));
	}
	current = config_setting_get_member(spin, optionName[AZIMUTH]);
	if (current) {
		getLimits(current, limit->azimuth);
	} else {
		memcpy(limit->azimuth, defaults->azimuth, MINMAX * sizeof(double));
	}
}

static void getSpins(config_setting_t *source, SourceLimits *defaults, SourceLimits *limit) {
	for (ushort blackhole = 0; blackhole < NUMBER_OF_BLACKHOLES; blackhole++) {
		config_setting_t *spin = config_setting_get_member(source, optionName[SPIN1 + blackhole]);
		if (spin) {
			getSpin(spin, &defaults->spin[blackhole], &limit->spin[blackhole]);
		} else {
			memcpy(&limit->spin[blackhole], &defaults->spin[blackhole],
				sizeof(defaults->spin[blackhole]));
		}
	}
}

static void getSourceParameters(config_setting_t *waveform, SourceLimits *defaults,
	SourceLimits *limit) {
	config_setting_t *source = config_setting_get_member(waveform, optionName[SOURCE]);
	if (source) {
		getMasses(source, defaults->mass, limit->mass);
		getSpins(source, defaults, limit);
		config_setting_t *current;
		current = config_setting_get_member(source, optionName[INCLINATION]);
		if (current) {
			getLimits(current, limit->inclination);
		} else {
			memcpy(defaults->inclination, limit->inclination, MINMAX * sizeof(double));
		}
		current = config_setting_get_member(source, optionName[DISTANCE]);
		if (current) {
			getLimits(current, limit->distance);
		} else {
			memcpy(defaults->distance, limit->distance, MINMAX * sizeof(double));
		}
	} else {
		memcpy(limit, defaults, sizeof(SourceLimits));
	}
}

static void getGenerationParameters(config_setting_t *waveform, Limits *defaults, Limits *limit) {
	config_setting_t *generation = config_setting_get_member(waveform, optionName[GENERATION]);
	if (generation) {
		cstring approximant = NULL, phase = NULL, spin = NULL, amplitude = NULL;
		config_setting_lookup_string(generation, optionName[APPROXIMANT], &approximant);
		if (approximant) {
			strcpy(limit->approximant, approximant);
		} else {
			strcpy(limit->approximant, defaults->approximant);
		}
		config_setting_lookup_string(generation, optionName[PHASE], &phase);
		if (phase) {
			strcpy(limit->phase, phase);
		} else {
			strcpy(limit->phase, defaults->phase);
		}
		config_setting_lookup_string(generation, optionName[SPIN], &spin);
		if (spin) {
			strcpy(limit->spin, spin);
		} else {
			strcpy(limit->spin, defaults->spin);
		}
		config_setting_lookup_string(generation, optionName[AMPLITUDE], &amplitude);
		if (amplitude) {
			strcpy(limit->amplitude, amplitude);
		} else {
			strcpy(limit->amplitude, defaults->amplitude);
		}
	} else {
		strcpy(limit->approximant, defaults->approximant);
		strcpy(limit->phase, defaults->phase);
		strcpy(limit->spin, defaults->spin);
		strcpy(limit->amplitude, defaults->amplitude);
	}
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
	puts("");
	// pairs
	Limits *pairsLimit = NULL;
	config_setting_t *current;
	config_setting_t *pairs = config_lookup(&cfg, optionName[PAIRS]);
	size_t numberOfPairs = (size_t) config_setting_length(pairs);
	pairsLimit = calloc(2 * numberOfPairs, sizeof(Limits));
	for (size_t i = 0; i < numberOfPairs; i++) {
		current = config_setting_get_elem(pairs, i);
		getWavePairParameters(current, &limit, &pairsLimit[2 * i]);
		printLimits(stderr, &pairsLimit[0]);
		//printLimits(stderr, &pairsLimit[1]);
	}
	// signal + templates
	/*Limits signalLimit;
	 config_setting_t *signal = config_lookup(&cfg, optionName[SIGNAL]);
	 getWaveformParameters(signal, &limit, &signalLimit);
	 Limits *templatesLimit = NULL;
	 config_setting_t *templates = config_lookup(&cfg, optionName[TEMPLATES]);
	 size_t numberOfTemplates = (size_t) config_setting_length(templates);
	 templatesLimit = calloc(numberOfTemplates, sizeof(Limits));
	 for (size_t i = 0; i < numberOfTemplates; i++) {
	 current = config_setting_get_elem(templates, i);
	 getWaveformParameters(current, &limit, &templatesLimit[i]);
	 }*/
	config_destroy(&cfg);
}
