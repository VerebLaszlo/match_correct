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

char const * optionName[] =
	{ "boundaryFrequency", "samplingFrequency", "default", "binary", "mass1", "mass2", "spin1",
		"spin2", "magnitude", "inclination", "azimuth", "distance", "detector", "generation",
		"approximant", "phase", "spin", "amplitude", "name", "pairs", "signal", "templates", };

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

static void getMasses(config_setting_t *binary, massLimits *defaults, massLimits *limit) {
	config_setting_t *mass = config_setting_get_member(binary, optionName[MASS1]);
	if (mass) {
		getLimits(mass, limit->mass[0]);
	} else {
		memcpy(limit->mass[0], defaults->mass[0], 2 * sizeof(defaults[0]));
	}
	mass = config_setting_get_member(binary, optionName[MASS2]);
	if (mass) {
		getLimits(mass, limit->mass[1]);
	} else {
		memcpy(limit->mass[1], defaults->mass[1], 2 * sizeof(defaults[0]));
	}
	limit->mode = GEN_M1M2;
}

static void getSpin(config_setting_t *spin, spinLimits *defaults, spinLimits *limit) {
	config_setting_t *current;
	current = config_setting_get_member(spin, optionName[MAGNITUDE]);
	if (current) {
		getLimits(current, limit->magnitude);
	} else {
		memcpy(limit->magnitude, defaults->magnitude, MINMAX * sizeof(double));
	}
	current = config_setting_get_member(spin, optionName[INCLINATION]);
	if (current) {
		getLimits(current, limit->inclination[PRECESSING]);
	} else {
		memcpy(limit->inclination, defaults->inclination, MINMAX * sizeof(double));
	}
	current = config_setting_get_member(spin, optionName[AZIMUTH]);
	if (current) {
		getLimits(current, limit->azimuth[PRECESSING]);
	} else {
		memcpy(limit->azimuth, defaults->azimuth, MINMAX * sizeof(double));
	}
	limit->mode = GEN_PRECESSING_ANGLES;
}

static void getSpins(config_setting_t *binary, binaryLimits *defaults, binaryLimits *limit) {
	for (ushort blackhole = 0; blackhole < NUMBER_OF_BLACKHOLES; blackhole++) {
		config_setting_t *spin = config_setting_get_member(binary, optionName[SPIN1 + blackhole]);
		if (spin) {
			getSpin(spin, &defaults->spin[blackhole], &limit->spin[blackhole]);
		} else {
			memcpy(&limit->spin[blackhole], &defaults->spin[blackhole],
				sizeof(defaults->spin[blackhole]));
		}
	}
}

static void getSourceParameters(config_setting_t *waveform, binaryLimits *defaults,
	binaryLimits *limit) {
	config_setting_t *binary = config_setting_get_member(waveform, optionName[BINARY]);
	if (binary) {
		getMasses(binary, &defaults->mass, &limit->mass);
		getSpins(binary, defaults, limit);
		config_setting_t *current;
		current = config_setting_get_member(binary, optionName[INCLINATION]);
		if (current) {
			getLimits(current, limit->inclination);
		} else {
			memcpy(defaults->inclination, limit->inclination, MINMAX * sizeof(double));
		}
		current = config_setting_get_member(binary, optionName[DISTANCE]);
		if (current) {
			getLimits(current, limit->distance);
		} else {
			memcpy(defaults->distance, limit->distance, MINMAX * sizeof(double));
		}
	} else {
		memcpy(limit, defaults, sizeof(binaryLimits));
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
	getSourceParameters(waveform, &defaults->binary, &limit->binary);
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

size_t getWaveformPairLimitsFrom(cstring fileName, Limits **pairsLimit) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, fileName)) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	config_setting_t *defaultWave = config_lookup(&cfg, optionName[DEFAULT]);
	Limits limit;
	getWaveformParameters(defaultWave, NULL, &limit);
	config_setting_t *current;
	config_setting_t *pairs = config_lookup(&cfg, optionName[PAIRS]);
	size_t numberOfPairs = (size_t) config_setting_length(pairs);
	(*pairsLimit) = calloc(2 * numberOfPairs, sizeof(Limits));
	for (size_t i = 0; i < numberOfPairs; i++) {
		current = config_setting_get_elem(pairs, i);
		getWavePairParameters(current, &limit, &(*pairsLimit)[2 * i]);
	}
	config_destroy(&cfg);
	return numberOfPairs;
}

size_t getSignalAndTemplatesLimitsFrom(cstring fileName, Limits **waveformLimit) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, "parser.conf")) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	config_setting_t *defaultWave = config_lookup(&cfg, optionName[DEFAULT]);
	Limits limit;
	getWaveformParameters(defaultWave, NULL, &limit);
	// signal + templates
	config_setting_t *signal = config_lookup(&cfg, optionName[SIGNAL]);
	config_setting_t *templates = config_lookup(&cfg, optionName[TEMPLATES]);
	size_t numberOfTemplates = (size_t) config_setting_length(templates);
	(*waveformLimit) = calloc(numberOfTemplates + 1, sizeof(Limits));
	getWaveformParameters(signal, &limit, &(*waveformLimit)[0]);
	config_setting_t *current;
	for (size_t i = 0; i < numberOfTemplates; i++) {
		current = config_setting_get_elem(templates, i);
		getWaveformParameters(current, &limit, &(*waveformLimit)[i + 1]);
	}
	config_destroy(&cfg);
	return numberOfTemplates + 1;
}
