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
	DIFF,
	NUMBER,
	WAVEX,
	PAIR,
	STEP,
	OPTIONS,
};

/** Names of the options. */
char optionName[OPTIONS][STRING_LENGTH] = {
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
    "diff",
    "number",
    "wave",
    "pair",
    "step" };

/** Structure containing the options hierarchy. */
typedef struct {
	cfg_opt_t units[4];	///< Unit options.
	cfg_opt_t spin[5];	///< Second spin parameters.
	cfg_opt_t binary[5];	///< Second spin parameters.
	cfg_opt_t method[4];	///< Generation method.
	cfg_opt_t defaultWave[5];	///< Default parameters.
	cfg_opt_t pair[2];	///< Default parameters.
	cfg_opt_t step[2];	///< Default parameters.
	cfg_opt_t option[7];	///< Group of the unit options.
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
	for (int blackhole = 0; blackhole < BH; blackhole++) {
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
	for (int blackhole = MIN; blackhole < BH; blackhole++) {
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
	parameters->number = cfg_getint(config, optionName[NUMBER]);
	for (int correct = 0; correct < 2; correct++) {
		parameters->diff[correct] = cfg_getnfloat(config, optionName[DIFF], correct);
	}
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
	free((*pair)->wave);
	free((*pair)->name);
	free(*pair);
}

void initParser(void) {
	memset(&defaultWave, 0, sizeof(Wave));
	sprintf(defaultWave.name, "wave");
	defaultWave.number = 1;
	sprintf(defaultWave.method.spin, "ALL");
	defaultWave.method.phase = 4;
	defaultWave.method.amplitude = 2;
	defaultWave.binary.distance = 1.0;
	defaultWave.binary.mass[0] = 3.0;
	defaultWave.binary.mass[1] = 3.0;
}

static int createList(double first, double second, char *text) {
	sprintf(text, "{%g, %g}", first, second);
	return (SUCCESS);
}

static int parsePair(cfg_t *config, Wave wave[]) {
	int failure = SUCCESS;
	for (size_t current = 0; current < cfg_size(config, optionName[WAVEX]); current++) {
		cfg_t *waveConfig = cfg_getnsec(config, optionName[WAVEX], current);
		failure &= parseWave(waveConfig, &wave[current]);
	}
	return (failure);
}

Option option = {	//
        { CFG_STR(optionName[ANGLE], "deg", CFGF_NONE),
        CFG_STR(optionName[MASS], "solar", CFGF_NONE),
        CFG_STR(optionName[DISTANCE], "Mpc", CFGF_NONE),
        CFG_END()
    }, {
        CFG_FLOAT_LIST(optionName[MAGNITUDE], "{1.0, 1.0}", CFGF_NONE),
        CFG_FLOAT_LIST(optionName[INCLINATION], "{0.0, 0.0}", CFGF_NONE),
        CFG_FLOAT_LIST(optionName[AZIMUTH], "{0.0, 0.0}", CFGF_NONE),
        CFG_STR(optionName[COORDINATE_SYSTEM], "precessing", CFGF_NONE),
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
        CFG_FLOAT_LIST(optionName[DIFF], "{0.1, 0.1}", CFGF_NONE),
        CFG_INT(optionName[NUMBER], 1, CFGF_NONE),
        CFG_END()
    }, {
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_MULTI),
        CFG_END()
    }, {
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_MULTI),
        CFG_END()
    }, {
        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_TITLE | CFGF_MULTI),
        CFG_SEC(optionName[PAIR], option.pair, CFGF_TITLE | CFGF_MULTI),
        CFG_SEC(optionName[STEP], option.step, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    }
};

static int parse(char *file, Parameter *parameters) {
	int failure = SUCCESS;
	cfg_t *config = cfg_init(option.option, CFGF_NONE);
	failure = cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		failure = parseFrequency(config, parameters);
		for (size_t current = 0; current < cfg_size(config, optionName[WAVEX]); current++) {
			cfg_t *wave = cfg_getsec(config, optionName[WAVEX]);
			if (strstr("default", cfg_title(wave))) {
				failure &= parseWave(wave, &defaultWave);
				break;
			}
		}
	}
	cfg_free(config);
	return (failure);
}

int parseWaves(char *file, Parameter *parameter) {
	int failure = SUCCESS;
	failure = parse(file, parameter);
	string mass, magnitude, inclination, azimuth, diff;
	createList(defaultWave.binary.mass[0], defaultWave.binary.mass[1], mass);
	createList(defaultWave.binary.spin.magnitude[0], defaultWave.binary.spin.magnitude[1], magnitude);
	createList(degreeFromRadian(defaultWave.binary.spin.inclination[0]),
	        degreeFromRadian(defaultWave.binary.spin.inclination[1]), inclination);
	createList(degreeFromRadian(defaultWave.binary.spin.azimuth[0]),
	        degreeFromRadian(defaultWave.binary.spin.azimuth[1]), azimuth);
	createList(defaultWave.diff[0], defaultWave.diff[1], diff);
	cfg_opt_t spin[5] = { //
	        CFG_FLOAT_LIST(optionName[MAGNITUDE], magnitude, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[INCLINATION], inclination, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[AZIMUTH], azimuth, CFGF_NONE),
	        CFG_STR(optionName[COORDINATE_SYSTEM], "precessing", CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t binary[5] = { //
	        CFG_FLOAT_LIST(optionName[MASS], mass, CFGF_NONE),
	        CFG_SEC(optionName[SPIN], spin, CFGF_NONE),
	        CFG_FLOAT(optionName[INCLINATION], defaultWave.binary.inclination, CFGF_NONE),
	        CFG_FLOAT(optionName[DISTANCE], defaultWave.binary.distance, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t method[4] = { //
	        CFG_STR(optionName[SPIN], defaultWave.method.spin, CFGF_NONE),
	        CFG_INT(optionName[PHASE], defaultWave.method.phase, CFGF_NONE),
	        CFG_INT(optionName[AMPLITUDE], defaultWave.method.amplitude, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t wave[5] = { //
	        CFG_SEC(optionName[BINARY], binary, CFGF_NONE),
	        CFG_SEC(optionName[METHOD], method, CFGF_NONE),
	        CFG_INT(optionName[NUMBER], 1, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[DIFF], diff, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t pair[2] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_END()
        };
	cfg_opt_t step[2] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_END()
        };
	cfg_opt_t options[7] = {	//
	        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
	        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
	        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[PAIR], pair, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[STEP], step, CFGF_TITLE | CFGF_MULTI),
	        CFG_END()
        };
	cfg_t *config = cfg_init(options, CFGF_NONE);
	failure = cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		parameter->exact = createWavePair(cfg_size(config, optionName[PAIR]));
		for (size_t current = 0; current < parameter->exact->length; current++) {
			cfg_t *pair = cfg_getnsec(config, optionName[PAIR], current);
			sprintf(parameter->exact->name[current], "%s", cfg_title(pair));
			failure |= parsePair(pair, &parameter->exact->wave[2 * current]);
		}
	}
	cfg_free(config);
	return (failure);
}

static int parseStep(char *file, Parameter *parameter) {
	int failure = SUCCESS;
	failure = parse(file, parameter);
	string magnitude, inclination, azimuth;
	createList(defaultWave.binary.spin.magnitude[0], defaultWave.binary.spin.magnitude[1], magnitude);
	createList(degreeFromRadian(defaultWave.binary.spin.inclination[0]),
	        degreeFromRadian(defaultWave.binary.spin.inclination[1]), inclination);
	createList(degreeFromRadian(defaultWave.binary.spin.azimuth[0]),
	        degreeFromRadian(defaultWave.binary.spin.azimuth[1]), azimuth);
	cfg_opt_t spin[5] = { //
	        CFG_FLOAT_LIST(optionName[MAGNITUDE], magnitude, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[INCLINATION], inclination, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[AZIMUTH], azimuth, CFGF_NONE),
	        CFG_STR(optionName[COORDINATE_SYSTEM], "precessing", CFGF_NONE),
	        CFG_END()
        };
	string mass;
	createList(defaultWave.binary.mass[0], defaultWave.binary.mass[1], mass);
	cfg_opt_t binary[5] = { //
	        CFG_FLOAT_LIST(optionName[MASS], mass, CFGF_NONE),
	        CFG_SEC(optionName[SPIN], spin, CFGF_NONE),
	        CFG_FLOAT(optionName[INCLINATION], defaultWave.binary.inclination, CFGF_NONE),
	        CFG_FLOAT(optionName[DISTANCE], defaultWave.binary.distance, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t method[4] = { //
	        CFG_STR(optionName[SPIN], defaultWave.method.spin, CFGF_NONE),
	        CFG_INT(optionName[PHASE], defaultWave.method.phase, CFGF_NONE),
	        CFG_INT(optionName[AMPLITUDE], defaultWave.method.amplitude, CFGF_NONE),
	        CFG_END()
        };
	string diff;
	createList(defaultWave.diff[0], defaultWave.diff[1], diff);
	cfg_opt_t wave[5] = { //
	        CFG_SEC(optionName[BINARY], binary, CFGF_NONE),
	        CFG_SEC(optionName[METHOD], method, CFGF_NONE),
	        CFG_INT(optionName[NUMBER], 1, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[DIFF], diff, CFGF_NONE),
	        CFG_END()
        };
	cfg_opt_t pair[2] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_END()
        };
	cfg_opt_t step[2] = {	//
	        CFG_SEC(optionName[WAVEX], wave, CFGF_MULTI),
	        CFG_END()
        };
	cfg_opt_t options[7] = {	//
	        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
	        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
	        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
	        CFG_SEC(optionName[WAVEX], option.defaultWave, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[PAIR], pair, CFGF_TITLE | CFGF_MULTI),
	        CFG_SEC(optionName[STEP], step, CFGF_TITLE | CFGF_MULTI),
	        CFG_END()
        };
	cfg_t *config = cfg_init(options, CFGF_NONE);
	failure = cfg_parse(config, file) == CFG_PARSE_ERROR;
	if (!failure) {
		parameter->step = createWavePair(cfg_size(config, optionName[WAVEX]) - 1);
		for (size_t current = 0, index = 0; current < parameter->step->length; current++) {
			cfg_t *step = cfg_getnsec(config, optionName[WAVEX], current);
			if (!strstr("default", cfg_title(step))) {
				sprintf(parameter->step->name[current], "%s", cfg_title(step));
				failure |= parseWave(step, &parameter->step->wave[2 * index]);
				index++;
			}
		}
	}
	cfg_free(config);
	return (failure);
}

void cleanParameter(Parameter *parameter) {
	destroyWavePair(&parameter->exact);
}

static int printWaveParameter(FILE *file, Wave *wave) {
	fprintf(file, "%11.5s %11.0u\n", wave->name, wave->number);
	fprintf(file, "%11.5s % 11.0d % 11.0d\n", wave->method.spin, wave->method.phase, wave->method.amplitude);
	fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g\n", wave->binary.mass[0], wave->binary.mass[1],
	        wave->binary.inclination, wave->binary.distance);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
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
	return (failure);
}
