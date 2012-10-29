/**	@file   parser_confuse.c
 *	@author László Veréb
 *	@date   29.08.2012
 *	@brief  Configuration file parser.
 */

#include <confuse.h>
#include <string.h>
#include <stdlib.h>
#include "util_math.h"
#include "parser_confuse.h"

/** IDs for the names of the options. */
enum {
	OUTPUT,
	ANGLE,
	MASS,
	DISTANCE,
	UNIT,
	BOUNDARY_FREQUENCY,
	SAMPLING_FREQUENCY,
	MAGNITUDE,
	INCLINATION,
	AZIMUTH,
	COORDINATE_SYSTEM,
	SPIN,
	BINARY,
	PHASE,
	AMPLITUDE,
	METHOD,
	WAVEX,
	PAIR,
	DIFF,
	GENERATE,
	STEP,
	OPTIONS,
};

/** Names of the options. */
char optionName[OPTIONS][STRING_LENGTH] = {
    "output",
    "angle",
    "mass",
    "distance",
    "units",
    "boundaryFrequency",
    "samplingFrequency",
    "magnitude",
    "inclination",
    "azimuth",
    "coorSystem",
    "spin",
    "binary",
    "phase",
    "amplitude",
    "method",
    "wave",
    "pair",
    "diff",
    "gen",
    "step" };

enum {
	UNIT_SIZE = 4,
	SPIN_SIZE = 5,
	NUMBER_POSITION = 3,
	BINARY_SIZE = 5,
	METHOD_SIZE = 4,
	WAVE_SIZE = 3,
	PAIR_SIZE = 2,
	STEP_SIZE = 4,
	OPTION_SIZE = 8,
};

/** Structure containing the options hierarchy. */
typedef struct {
	cfg_opt_t units[UNIT_SIZE];	///< Unit options.
	cfg_opt_t spin[SPIN_SIZE];	///< Second spin parameters.
	cfg_opt_t binary[BINARY_SIZE];	///< Second spin parameters.
	cfg_opt_t method[METHOD_SIZE];	///< Generation method.
	cfg_opt_t defaultWave[WAVE_SIZE];	///< Default parameters.
	cfg_opt_t pair[PAIR_SIZE];	///< Default parameters.
	cfg_opt_t step[STEP_SIZE];	///< Default parameters.
	cfg_opt_t option[OPTION_SIZE];	///< Group of the unit options.
} Option;

Wave defaultWave;	///< default wave parameters.

static int parseFrequency(cfg_t *config, Parameter *parameters) {
	parameters->initialFrequency = cfg_getnfloat(config, optionName[BOUNDARY_FREQUENCY], 0);
	parameters->endingFrequency = cfg_getnfloat(config, optionName[BOUNDARY_FREQUENCY], 1);
	parameters->samplingFrequency = cfg_getfloat(config, optionName[SAMPLING_FREQUENCY]);
	parameters->samplingTime = 1.0 / parameters->samplingFrequency;
	return (SUCCESS);
}

static int parseSpin(cfg_t *config, Spin *spin) {
	for (int blackhole = FIRST; blackhole < BH; blackhole++) {
		spin->magnitude[blackhole] = cfg_getnfloat(config, optionName[MAGNITUDE], blackhole);
		spin->inclination[blackhole] = cfg_getnfloat(config, optionName[INCLINATION], blackhole);
		spin->inclination[blackhole] = radianFromDegree(spin->inclination[blackhole]);
		spin->azimuth[blackhole] = cfg_getnfloat(config, optionName[AZIMUTH], blackhole);
		spin->azimuth[blackhole] = radianFromDegree(spin->azimuth[blackhole]);
	}
	spin->system = PRECESSING;
	return (SUCCESS);
}

static int parseGeneration(cfg_t *config, Method *method) {
	char *spin = cfg_getstr(config, optionName[SPIN]);
	strcpy(method->spin, spin);
	method->phase = cfg_getint(config, optionName[PHASE]);
	method->amplitude = cfg_getint(config, optionName[AMPLITUDE]);
	return (SUCCESS);
}

static int parseBinary(cfg_t *config, Binary *binary) {
	int failure = SUCCESS;
	cfg_t *spin = cfg_getsec(config, optionName[SPIN]);
	for (int blackhole = FIRST; blackhole < BH; blackhole++) {
		binary->mass[blackhole] = cfg_getnfloat(config, optionName[MASS], blackhole);
	}
	binary->inclination = cfg_getfloat(config, optionName[INCLINATION]);
	binary->inclination = radianFromDegree(binary->inclination);
	binary->distance = cfg_getfloat(config, optionName[DISTANCE]);
	failure = parseSpin(spin, &binary->spin);
	return (failure);
}

static int parseWave(cfg_t *config, Wave *parameters) {
	int failure = SUCCESS;
	cfg_t *binary = cfg_getsec(config, optionName[BINARY]);
	failure &= parseBinary(binary, &parameters->binary);
	cfg_t *generation = cfg_getsec(config, optionName[METHOD]);
	failure &= parseGeneration(generation, &parameters->method);
	return (failure);
}

static WavePair *createWavePair(size_t length) {
	WavePair *pair = calloc(1, sizeof(WavePair));
	pair->length = length;
	pair->wave = calloc(2 * pair->length, sizeof(Wave));
	pair->name = calloc(pair->length, sizeof(string));
	return (pair);
}

static void destroyWavePair(WavePair **pair) {
	if (*pair) {
		if ((*pair)->wave) {
			free((*pair)->wave);
		}
		if ((*pair)->name) {
			free((*pair)->name);
		}
		free(*pair);
	}
}

static int createList(double first, double second, char *text) {
	sprintf(text, "{%g, %g}", first, second);
	return (SUCCESS);
}

static int parsePair(cfg_t *config, Wave wave[]) {
	int failure = SUCCESS;
	for (size_t current = FIRST; current < cfg_size(config, optionName[WAVEX]); current++) {
		cfg_t *waveConfig = cfg_getnsec(config, optionName[WAVEX], current);
		failure &= parseWave(waveConfig, &wave[current]);
	}
	return (failure);
}

#define outputConstant "out"
#define samplingFrequencyConstant 10240
#define boundaryFrequencyConstant "{20.0, 2000.0}"
#define coordinateSystemConstant "precessing"
#define numberConstant 1
#define differenceConstant "{2, 2}"
#define genConstant "{true, true, true, true}"

Option option = {	//
        { CFG_STR(optionName[ANGLE], "deg", CFGF_NONE),
        CFG_STR(optionName[MASS], "solar", CFGF_NONE),
        CFG_STR(optionName[DISTANCE], "Mpc", CFGF_NONE),
        CFG_END()
    }, {
        CFG_FLOAT_LIST(optionName[MAGNITUDE], "{1.0, 1.0}", CFGF_NONE),
        CFG_FLOAT_LIST(optionName[INCLINATION], "{0.0, 0.0}", CFGF_NONE),
        CFG_FLOAT_LIST(optionName[AZIMUTH], "{0.0, 0.0}", CFGF_NONE),
        CFG_STR(optionName[COORDINATE_SYSTEM], coordinateSystemConstant, CFGF_NONE),
        CFG_END()
    }, {
        CFG_FLOAT_LIST(optionName[MASS], "{3.0, 3.0}", CFGF_NONE),
        CFG_SEC(optionName[SPIN], option.spin, CFGF_NONE),
        CFG_FLOAT(optionName[INCLINATION], 10.0, CFGF_NONE),
        CFG_FLOAT(optionName[DISTANCE], 1.0, CFGF_NONE),
        CFG_END()
    }, {
        CFG_STR_LIST(optionName[SPIN], "{ALL, ALL}", CFGF_NONE),
        CFG_INT_LIST(optionName[PHASE], "{4, 4}", CFGF_NONE),
        CFG_INT_LIST(optionName[AMPLITUDE], "{0, 0}", CFGF_NONE),
        CFG_END()
    }, {
        CFG_SEC(optionName[BINARY], option.binary, CFGF_NONE),
        CFG_SEC(optionName[METHOD], option.method, CFGF_NONE),
        CFG_END()
    }, {
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_MULTI),
        CFG_END()
    }, {
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_MULTI),
        CFG_INT_LIST(optionName[DIFF], differenceConstant, CFGF_NONE),
        CFG_BOOL_LIST(optionName[GENERATE], genConstant, CFGF_NONE),
        CFG_END()
    }, {
        CFG_STR(optionName[OUTPUT], outputConstant, CFGF_NONE),
        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], boundaryFrequencyConstant, CFGF_NONE),
        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], samplingFrequencyConstant, CFGF_NONE),
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_TITLE | CFGF_MULTI),
        CFG_SEC(optionName[PAIR], option.pair, CFGF_TITLE | CFGF_MULTI),
        CFG_SEC(optionName[STEP], option.step, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    }
};

static int parse(char *file, Parameter *parameters) {
	parameters->exactTrue = parameters->stepTrue = false;
	int failure = SUCCESS;
	cfg_t *config = cfg_init(option.option, CFGF_NONE);
	failure = cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		failure = parseFrequency(config, parameters);
		for (size_t current = FIRST; current < cfg_size(config, optionName[WAVEX]); current++) {
			cfg_t *wave = cfg_getsec(config, optionName[WAVEX]);
			if (strstr("default", cfg_title(wave))) {
				parameters->exactTrue = true;
				failure &= parseWave(wave, &defaultWave);
				break;
			}
		}
		for (size_t current = FIRST; current < cfg_size(config, optionName[STEP]); current++) {
			cfg_t *step = cfg_getsec(config, optionName[STEP]);
			if (strstr("default", cfg_title(step))) {
				parameters->stepTrue = true;
				failure &= parsePair(step, parameters->boundary);
				for (int current = FIRST; current < BH; current++) {
					parameters->numberOfStep[current] = cfg_getnint(step, optionName[DIFF], current);
					if (parameters->numberOfStep[current] < 2) {
						parameters->numberOfStep[current] = 2;
					}
				}
				for (size_t current = FIRST; current < cfg_size(step, optionName[GENERATE]); current++) {
					parameters->gen[current] = cfg_getnbool(step, optionName[GENERATE], current);
				}
				break;
			}
		}
	}
	cfg_free(config);
	return (failure);
}

static int initOptions(char *file, Parameter *parameter, cfg_t **config) {
	int failure = parse(file, parameter);
	string magnitude, inclination, azimuth;
	createList(defaultWave.binary.spin.magnitude[FIRST], defaultWave.binary.spin.magnitude[SECOND], magnitude);
	createList(degreeFromRadian(defaultWave.binary.spin.inclination[FIRST]),
	        degreeFromRadian(defaultWave.binary.spin.inclination[SECOND]), inclination);
	createList(degreeFromRadian(defaultWave.binary.spin.azimuth[FIRST]),
	        degreeFromRadian(defaultWave.binary.spin.azimuth[SECOND]), azimuth);
	cfg_opt_t spin[SPIN_SIZE] = { //
	        CFG_FLOAT_LIST(optionName[MAGNITUDE], magnitude, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[INCLINATION], inclination, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[AZIMUTH], azimuth, CFGF_NONE),
	        CFG_STR(optionName[COORDINATE_SYSTEM], coordinateSystemConstant, CFGF_NONE),
	        CFG_END()
        };
	string mass;
	createList(defaultWave.binary.mass[FIRST], defaultWave.binary.mass[SECOND], mass);
	cfg_opt_t binary[BINARY_SIZE] = { //
	        CFG_FLOAT_LIST(optionName[MASS], mass, CFGF_NONE),
	        CFG_SEC(optionName[SPIN], spin, CFGF_NONE),
	        CFG_FLOAT(optionName[INCLINATION], defaultWave.binary.inclination, CFGF_NONE),
	        CFG_FLOAT(optionName[DISTANCE], defaultWave.binary.distance, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t method[METHOD_SIZE] = { //
	        CFG_STR(optionName[SPIN], defaultWave.method.spin, CFGF_NONE),
	        CFG_INT(optionName[PHASE], defaultWave.method.phase, CFGF_NONE),
	        CFG_INT(optionName[AMPLITUDE], defaultWave.method.amplitude, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t wave[WAVE_SIZE] = { //
	        CFG_SEC(optionName[BINARY], binary, CFGF_NONE),
	        CFG_SEC(optionName[METHOD], method, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t pair[PAIR_SIZE] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_END()
        };
	cfg_opt_t step[STEP_SIZE] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_INT_LIST(optionName[DIFF], differenceConstant, CFGF_NONE),
	        CFG_BOOL_LIST(optionName[GENERATE], genConstant, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t options[OPTION_SIZE] = {	//
	        CFG_STR(optionName[OUTPUT], outputConstant, CFGF_NONE),
	        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], boundaryFrequencyConstant, CFGF_NONE),
	        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], samplingFrequencyConstant, CFGF_NONE),
	        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[PAIR], pair, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[STEP], step, CFGF_TITLE | CFGF_MULTI),
	        CFG_END()
        };
	*config = cfg_init(options, CFGF_NONE);
	return (failure);
}

cfg_t *config;

int initParser(char *file, Parameter *parameter, string outputDir) {
	memset(&defaultWave, 0, sizeof(Wave));
	strcpy(outputDir, outputConstant);
	strcpy(defaultWave.name, "wave");
	strcpy(defaultWave.method.spin, "ALL");
	defaultWave.method.phase = 4;
	defaultWave.method.amplitude = 2;
	defaultWave.binary.distance = 1.0;
	defaultWave.binary.mass[FIRST] = 3.0;
	defaultWave.binary.mass[SECOND] = 3.0;
	int failure = initOptions(file, parameter, &config);
	failure &= cfg_parse(config, file) == CFG_PARSE_ERROR;
	char *output = cfg_getstr(config, optionName[OUTPUT]);
	strcpy(outputDir, output);
	return (failure);
}

int parseWaves(char *file, Parameter *parameter) {
	int failure = SUCCESS;
	failure &= cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		parameter->exact = createWavePair(cfg_size(config, optionName[PAIR]));
		for (size_t current = FIRST; current < parameter->exact->length; current++) {
			cfg_t *pair = cfg_getnsec(config, optionName[PAIR], current);
			sprintf(parameter->exact->name[current], "%s", cfg_title(pair));
			failure |= parsePair(pair, &parameter->exact->wave[2 * current]);
		}
	}
	return (failure);
}

int parseStep(char *file, Parameter *parameter) {
	int failure = SUCCESS;
	failure &= cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		parameter->step = createWavePair(cfg_size(config, optionName[STEP]) - 1);
		for (size_t current = FIRST, index = FIRST; current < parameter->step->length + 1; current++) {
			cfg_t *step = cfg_getnsec(config, optionName[STEP], current);
			if (!strstr("default", cfg_title(step))) {
				sprintf(parameter->step->name[index], "%s", cfg_title(step));
				failure |= parsePair(step, &parameter->step->wave[2 * index]);
				index++;
			}
		}
	}
	return (failure);
}

void cleanParameter(Parameter *parameter) {
	destroyWavePair(&parameter->exact);
	destroyWavePair(&parameter->step);
	cfg_free(config);
}

static int printWaveParameter(FILE *file, Wave *wave) {
	fprintf(file, "%11.5s\n", wave->name);
	fprintf(file, "%11s % 11.0d % 11.0d\n", wave->method.spin, wave->method.phase, wave->method.amplitude);
	fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g\n", wave->binary.mass[0], wave->binary.mass[1],
	        degreeFromRadian(wave->binary.inclination), wave->binary.distance);
	for (int blackhole = FIRST; blackhole < BH; blackhole++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", wave->binary.spin.magnitude[blackhole],
		        degreeFromRadian(wave->binary.spin.inclination[blackhole]),
		        degreeFromRadian(wave->binary.spin.azimuth[blackhole]));
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", wave->binary.spin.component[blackhole][X],
		        wave->binary.spin.component[blackhole][Y], wave->binary.spin.component[blackhole][Z]);
	}
	return (SUCCESS);
}

int printParameter(FILE *file, Parameter *parameter, int from, int to) {
	int failure = SUCCESS;
	fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->initialFrequency, parameter->endingFrequency,
	        parameter->samplingFrequency);
	for (int current = from; current < to; current++) {
		failure |= printWaveParameter(file, &parameter->exact->wave[current]);
	}
	puts("Step:");
	for (int boundary = MIN; boundary < MINMAX; boundary++) {
		failure |= printWaveParameter(file, &parameter->boundary[boundary]);
	}
	for (int current = from; current < to; current++) {
		failure |= printWaveParameter(file, &parameter->step->wave[current]);
	}
	return (failure);
}
