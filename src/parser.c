/**
 * @file parser.c
 * @author vereb
 * @date Sep 28, 2011
 * @brief 
 */

#include <libconfig.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parser.h"

typedef enum {
	UNITS,
	ANGLE,
	MASS,
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

char const * optionName[] = { "units", "angle", "mass", "boundaryFrequency", "samplingFrequency",
								"default", "binary", "mass1", "mass2", "spin1", "spin2",
								"magnitude", "inclination", "azimuth", "distance", "detector",
								"generation", "approximant", "phase", "spin", "amplitude", "name",
								"pairs", "signal", "templates", };

static void getNameAndNumberOfRunsFrom(cstring code, string *name, size_t *numberOfRuns) {
	stringPointer other = NULL;
	cstring number = NULL;
	strcpy(*name, "");
	*numberOfRuns = 1;
	if (code[0] == '*') {
		number = &code[1];
	} else if (strlen(code) > 3) {
		string temp1;
		stringPointer temp2;
		strcpy(temp1, code);
		temp2 = strtok(temp1, "*");
		strcpy(*name, temp2);
		number = strtok(NULL, "*");
	}
	if (number) {
		*numberOfRuns = (size_t) strtol(number, &other, 10);
	}
}

typedef enum {
	UNIT_ERROR, RAD, DEGREE = RAD << 1, TURN = DEGREE << 1, SOLAR = TURN << 1, KILOGRAM = SOLAR << 1,
} Units;

Units units;

static void getUnitsCodeFromStrings(cstring angle, cstring mass) {
	units = UNIT_ERROR;
	if (!strcmp(angle, "rad")) {
		units |= RAD;
	} else if (!strcmp(angle, "degree")) {
		units |= DEGREE;
	} else if (!strcmp(angle, "turn")) {
		units |= TURN;
	}
	if (!strcmp(mass, "solar")) {
		units |= SOLAR;
	} else if (!strcmp(angle, "kg")) {
		units |= KILOGRAM;
	}
}

static void convertAngleToRadian(double *angle) {
	if (units & DEGREE) {
		*angle = radianFromDegree(*angle);
	} else if (units & TURN) {
		*angle = radianFromTurn(*angle);
	}
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
	for (ushort minimax = ZERO; minimax < MINMAX; minimax++) {
		limit->chirpMass[minimax] = limit->eta[minimax] = limit->m1_m2[minimax] =
			limit->mu[minimax] = limit->nu[minimax] = limit->totalMass[minimax] = NAN;
	}
}

static void getSpin(config_setting_t *spin, spinLimits *defaults, spinLimits *limit) {
	config_setting_t *current;
	for (ushort minimax = ZERO; minimax < MINMAX; minimax++) {
		for (ushort convention = ZERO; convention < COORDINATE_CONVENTIONS; convention++) {
			limit->inclination[convention][minimax] = limit->elevation[convention][minimax] = limit
				->azimuth[convention][minimax] = NAN;
			for (ushort dimention = X; dimention < DIMENSION; dimention++) {
				limit->component[convention][dimention][minimax] =
					limit->unity[convention][dimention][minimax] = NAN;
			}
		}
	}
	current = config_setting_get_member(spin, optionName[MAGNITUDE]);
	if (current) {
		getLimits(current, limit->magnitude);
	} else {
		memcpy(limit->magnitude, defaults->magnitude, MINMAX * sizeof(defaults->magnitude));
	}
	ushort size = COORDINATE_CONVENTIONS * MINMAX;
	current = config_setting_get_member(spin, optionName[INCLINATION]);
	if (current) {
		getLimits(current, limit->inclination[PRECESSING]);
		convertAngleToRadian(&limit->inclination[PRECESSING][MIN]);
		convertAngleToRadian(&limit->inclination[PRECESSING][MAX]);
	} else {
		memcpy(limit->inclination, defaults->inclination, size * sizeof(defaults->inclination));
	}
	current = config_setting_get_member(spin, optionName[AZIMUTH]);
	if (current) {
		getLimits(current, limit->azimuth[PRECESSING]);
		convertAngleToRadian(&limit->azimuth[PRECESSING][MIN]);
		convertAngleToRadian(&limit->azimuth[PRECESSING][MAX]);
	} else {
		memcpy(limit->azimuth, defaults->azimuth, size * sizeof(defaults->azimuth));
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
			convertAngleToRadian(&limit->inclination[MIN]);
			convertAngleToRadian(&limit->inclination[MAX]);
		} else {
			memcpy(limit->inclination, defaults->inclination, sizeof(defaults->inclination));
		}
		current = config_setting_get_member(binary, optionName[DISTANCE]);
		if (current) {
			getLimits(current, limit->distance);
		} else {
			memcpy(limit->distance, defaults->distance, MINMAX * sizeof(limit->distance));
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
	cstring name = NULL;
	config_setting_lookup_string(waveform, optionName[NAME], &name);
	if (name) {
		getNameAndNumberOfRunsFrom(name, &limit->name, &limit->numberOfRuns);
		if (defaults) {
			limit->numberOfRuns =
				limit->numberOfRuns < defaults->numberOfRuns ? limit->numberOfRuns :
					defaults->numberOfRuns;
		}
		if (strlen(limit->name) == 0) {
			strcpy(limit->name, defaults->name);
		}
	} else {
		strcpy(limit->name, defaults->name);
		limit->numberOfRuns = defaults->numberOfRuns;
	}
}

static void getWavePairParameters(config_setting_t *pair, Limits *defaults, Limits limit[]) {
	config_setting_t *current;
	ushort count = (ushort) neededElementNumber(3, pair);
	for (ushort i = 0; i < count - 1; i++) {
		current = config_setting_get_elem(pair, i);
		getWaveformParameters(current, defaults, &limit[i]);
	}
	cstring code = config_setting_get_string_elem(pair, count - 1);
	string name;
	size_t numberOfRuns;
	getNameAndNumberOfRunsFrom(code, &name, &numberOfRuns);
	if (defaults) {
		limit[MIN].numberOfRuns = limit[MAX].numberOfRuns =
			numberOfRuns < defaults->numberOfRuns ? numberOfRuns : defaults->numberOfRuns;
	}
	if (strlen(name) != 0) {
		strcpy(limit[MIN].name, name);
		strcpy(limit[MAX].name, name);
	} else {
		strcpy(limit[MIN].name, defaults->name);
		strcpy(limit[MAX].name, defaults->name);
	}
}

static bool getConstantParameters(ConstantParameters *constants, config_t *cfg) {
	bool succes = false;
	config_setting_t *unit = config_lookup(cfg, optionName[UNITS]);
	if (unit) {
		cstring angleCode, massCode;
		config_setting_lookup_string(unit, optionName[ANGLE], &angleCode);
		config_setting_lookup_string(unit, optionName[MASS], &massCode);
		getUnitsCodeFromStrings(angleCode, massCode);
		config_setting_t *frequency = config_lookup(cfg, optionName[BOUNDARY_FREQUENCY]);
		if (frequency) {
			constants->initialFrequency = config_setting_get_int_elem(frequency, MIN);
			constants->endingFrequency = config_setting_get_int_elem(frequency, MAX);
			frequency = config_lookup(cfg, optionName[SAMPLING_FREQUENCY]);
			if (frequency) {
				constants->samplingFrequency = config_setting_get_int(frequency);
				succes = true;
			}
		}
	}
	return succes;
}

Limits *createWaveformPairLimitsFrom(cstring fileName, ConstantParameters *constants,
	size_t *numberOfPairs) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, fileName)) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	bool succes = getConstantParameters(constants, &cfg);
	config_setting_t *defaultWave = config_lookup(&cfg, optionName[DEFAULT]);
	Limits *pairLimits = NULL;
	*numberOfPairs = 0;
	if (defaultWave && succes) {
		Limits limit;
		getWaveformParameters(defaultWave, NULL, &limit);
		config_setting_t *current;
		config_setting_t *pairs = config_lookup(&cfg, optionName[PAIRS]);
		if (pairs) {
			*numberOfPairs = (size_t) config_setting_length(pairs);
			pairLimits = calloc(2 * *numberOfPairs, sizeof(Limits));
			for (size_t i = 0; i < *numberOfPairs; i++) {
				current = config_setting_get_elem(pairs, i);
				getWavePairParameters(current, &limit, &pairLimits[2 * i]);
			}
		}
	}
	config_destroy(&cfg);
	return pairLimits;
}

void destroyWaveformPairLimits(Limits *pairs) {
	if (pairs) {
		free(pairs);
	}
}

Limits *createSignalAndTemplatesLimitsFrom(cstring fileName, ConstantParameters *constants,
	size_t *size) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, fileName)) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	bool succes = getConstantParameters(constants, &cfg);
	config_setting_t *defaultWave = config_lookup(&cfg, optionName[DEFAULT]);
	Limits limit;
	size_t numberOfTemplates = 0;
	*size = 0;
	Limits *waveformLimit = NULL;
	if (defaultWave && succes) {
		getWaveformParameters(defaultWave, NULL, &limit);
		config_setting_t *signal = config_lookup(&cfg, optionName[SIGNAL]);
		config_setting_t *templates = config_lookup(&cfg, optionName[TEMPLATES]);
		if (signal && templates) {
			numberOfTemplates = (size_t) config_setting_length(templates);
			*size = numberOfTemplates + 1;
			waveformLimit = calloc(*size, sizeof(Limits));
			getWaveformParameters(signal, &limit, &waveformLimit[0]);
			config_setting_t *current;
			for (size_t i = 0; i < numberOfTemplates; i++) {
				current = config_setting_get_elem(templates, i);
				getWaveformParameters(current, &limit, &waveformLimit[i + 1]);
			}
		}
	}
	config_destroy(&cfg);
	return waveformLimit;
}

void destroySignalAndTemplatesLimits(Limits *limits) {
	if (limits) {
		free(limits);
	}
}

static void getExactMasses(config_setting_t *binary, massParameters *mass) {
	config_setting_lookup_float(binary, optionName[MASS1], &mass->mass[0]);
	config_setting_lookup_float(binary, optionName[MASS2], &mass->mass[1]);
}

static void getExactSpin(config_setting_t *currentSpin, spinParameters *spin) {
	config_setting_lookup_float(currentSpin, optionName[MAGNITUDE], &spin->magnitude);
	config_setting_lookup_float(currentSpin, optionName[INCLINATION],
		&spin->inclination[PRECESSING]);
	config_setting_lookup_float(currentSpin, optionName[AZIMUTH], &spin->azimuth[PRECESSING]);
}

static void getExactSpins(config_setting_t *currentBinary, BinarySystem *binary) {
	for (ushort blackhole = 0; blackhole < NUMBER_OF_BLACKHOLES; blackhole++) {
		config_setting_t *currentSpin = config_setting_get_member(currentBinary,
			optionName[SPIN1 + blackhole]);
		getExactSpin(currentSpin, &binary->spin[blackhole]);
	}
}

static void getExactSourceParameters(config_setting_t *waveform, BinarySystem *binary) {
	config_setting_t *currentBinary = config_setting_get_member(waveform, optionName[BINARY]);
	if (currentBinary) {
		getExactMasses(currentBinary, &binary->mass);
		getExactSpins(currentBinary, binary);
		config_setting_lookup_float(currentBinary, optionName[INCLINATION], &binary->inclination);
		config_setting_lookup_float(currentBinary, optionName[DISTANCE], &binary->distance);
	}
}

static void getExactGenerationParameters(config_setting_t *waveform, ushort i,
	SystemParameter *systems) {
	config_setting_t *generation = config_setting_get_member(waveform, optionName[GENERATION]);
	if (generation) {
		cstring approximant = NULL, phase = NULL, spin = NULL, amplitude = NULL;
		config_setting_lookup_string(generation, optionName[APPROXIMANT], &approximant);
		strcpy(systems->approximant[i], approximant);
		config_setting_lookup_string(generation, optionName[PHASE], &phase);
		strcpy(systems->phase[i], phase);
		config_setting_lookup_string(generation, optionName[SPIN], &spin);
		strcpy(systems->spin[i], spin);
		config_setting_lookup_string(generation, optionName[AMPLITUDE], &amplitude);
		strcpy(systems->amplitude[i], amplitude);
	}
}

static void getExactWaveformParameters(config_setting_t *waveform, ushort i,
	SystemParameter *systems) {
	getExactSourceParameters(waveform, &systems->system[i]);
	getExactGenerationParameters(waveform, i, systems);
}

static void getExactWavePairParameters(config_setting_t *pair, SystemParameter *currentSystem) {
	config_setting_t *current;
	ushort count = (ushort) neededElementNumber(3, pair);
	for (ushort i = 0; i < count - 1; i++) {
		current = config_setting_get_elem(pair, i);
		getExactWaveformParameters(current, i, currentSystem);
	}
	cstring code = config_setting_get_string_elem(pair, count - 1);
	string name;
	size_t numberOfRuns;
	getNameAndNumberOfRunsFrom(code, &name, &numberOfRuns);
	strcpy(currentSystem->name[0], name);
	strcpy(currentSystem->name[1], name);
}

SystemParameter *createExactWaveformPairFrom(cstring fileName, ConstantParameters *constants,
	size_t *numberOfPairs) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, fileName)) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	bool succes = getConstantParameters(constants, &cfg);
	SystemParameter *parameters = NULL;
	*numberOfPairs = 0;
	if (succes) {
		config_setting_t *current;
		config_setting_t *pairs = config_lookup(&cfg, optionName[PAIRS]);
		if (pairs) {
			*numberOfPairs = (size_t) config_setting_length(pairs);
			parameters = calloc(2 * *numberOfPairs, sizeof(SystemParameter));
			for (size_t i = 0; i < *numberOfPairs; i++) {
				current = config_setting_get_elem(pairs, i);
				getExactWavePairParameters(current, &parameters[i]);
			}
		}
	}
	config_destroy(&cfg);
	return parameters;
}

void destroyExactWaveformPairs(SystemParameter *pairs) {
	if (pairs) {
		free(pairs);
	}
}

typedef enum {
	OUTPUT_DIRECTORY,
	NUMBER_OF_RUNS,
	SIGNAL_DATA_FORMAT,
	SIGNAL_FORMAT,
	DATA_FORMAT,
	PRECISION,
	WIDTH,
	SPECIFIER,
	SEPARATOR,
	LEFT_JUSTIFIED,
	NUMBER_OF_PROGRAM_OPTIONS,
} ProgramOptionCode;

char const * programOptionName[] = { "outputDirectory", "numberOfRuns", "signalDataFormat",
										"signalFormat", "dataFormat", "precision", "width",
										"specifier", "separator", "leftJustified", };

static void getFormat(OutputFormats code, config_setting_t *format) {
	if (format) {
		long precision, width;
		cstring specifier;
		cstring separator;
		int leftJustified;
		config_setting_lookup_int(format, programOptionName[PRECISION], &precision);
		config_setting_lookup_int(format, programOptionName[WIDTH], &width);
		config_setting_lookup_string(format, programOptionName[SPECIFIER], &specifier);
		config_setting_lookup_string(format, programOptionName[SEPARATOR], &separator);
		config_setting_lookup_bool(format, programOptionName[LEFT_JUSTIFIED], &leftJustified);
		setOutputFormat(&outputFormat[code], precision, width, specifier[0], separator[0],
			leftJustified);
	}
}

void getProgramParametersFrom(cstring fileName, ProgramParameter *parameters, Options *option) {
	config_t cfg;
	memset(&cfg, 0, sizeof(cfg));
	if (!config_read_file(&cfg, fileName)) {
		fprintf(stderr, "Error in %s config file: %d - %s\n", fileName, config_error_line(&cfg),
			config_error_text(&cfg));
		config_destroy(&cfg);
		exit(EXIT_FAILURE);
	}
	parameters->calculateMatches = option->calculateMatch;
	parameters->plot = option->plot;
	config_setting_t *setting = config_lookup(&cfg, programOptionName[OUTPUT_DIRECTORY]);
	cstring outputDirectoryName = config_setting_get_string(setting);
	strcpy(parameters->outputDirectory, outputDirectoryName);
	config_lookup_int(&cfg, programOptionName[NUMBER_OF_RUNS], &parameters->numberOfRuns);
	config_setting_t *format = config_lookup(&cfg, programOptionName[SIGNAL_DATA_FORMAT]);
	getFormat(SIGNAL_DATA, format);
	format = config_lookup(&cfg, programOptionName[SIGNAL_FORMAT]);
	getFormat(SIGNAL_TO_PLOT, format);
	format = config_lookup(&cfg, programOptionName[DATA_FORMAT]);
	getFormat(DATA, format);
	config_destroy(&cfg);
}
