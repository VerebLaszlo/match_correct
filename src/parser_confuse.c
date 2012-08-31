/**	@file	parser_confuse.c
 *	@author vereb
 *	@date	29.08.2012
 *	@brief
 */

#include <confuse.h>
#include <string.h>
#include "util_math.h"
#include "parser_confuse.h"

/** Various constants. */
enum {
	BASE_OPTIONS = 5,
};

/** IDs for the names of the options. */
enum {
	UNIT,
	ANGLE,
	MASS,
	DISTANCE,
	BOUNDARY_FREQUENCY,
	SAMPLING_FREQUENCY,
	MAGNITUDE,
	INCLINATION,
	AZIMUTH,
	COORDINATE_SYSTEM,
	SPIN,
	BINARY,
	APPROXIMANT,
	PHASE,
	AMPLITUDE,
	METHOD,
	NUMBER,
	NAME,
	DEFAULT,
	OPTIONS,
};

/** Names of the options. */
char optionName[OPTIONS][STRING_LENGTH] = {
    "units",
    "angle",
    "mass",
    "distance",
    "boundaryFrequency",
    "samplingFrequency",
    "magnitude",
    "inclination",
    "azimuth",
    "coorSystem",
    "spin",
    "binary",
    "approximant",
    "phase",
    "amplitude",
    "method",
    "number",
    "name",
    "default", };

/** Structure containing the options hierarchy. */
typedef struct {
	cfg_opt_t units[4];	///< Unit options.
	cfg_opt_t spin[5];	///< Second spin parameters.
	cfg_opt_t binary[5];	///< Second spin parameters.
	cfg_opt_t method[5];	///< Generation method.
	cfg_opt_t defaultWave[5];	///< Default parameters.
	cfg_opt_t option[BASE_OPTIONS];	///< Group of the unit options.
} Option;

/** %Option hierarchy. */
Option option = {
	{
		CFG_STR(optionName[ANGLE], "deg", CFGF_NONE),
		CFG_STR(optionName[MASS], "solar", CFGF_NONE),
		CFG_STR(optionName[DISTANCE], "Mpc", CFGF_NONE),
		CFG_END()
	}, {
		CFG_FLOAT_LIST(optionName[MAGNITUDE], "{1.0, 1.0}", CFGF_NONE),
		CFG_FLOAT_LIST(optionName[INCLINATION], "{180.0, 180.0}", CFGF_NONE),
		CFG_FLOAT_LIST(optionName[AZIMUTH], "{0.0, 0.0}", CFGF_NONE),
		CFG_STR(optionName[COORDINATE_SYSTEM], "precessing", CFGF_NONE),
		CFG_END()
	}, {
		CFG_FLOAT_LIST(optionName[MASS], "{30.0, 30.0}", CFGF_NONE),
		CFG_SEC(optionName[SPIN], option.spin, CFGF_NONE),
		CFG_FLOAT(optionName[INCLINATION], 10.0, CFGF_NONE),
		CFG_FLOAT(optionName[DISTANCE], 1.0, CFGF_NONE),
		CFG_END()
	}, {
		CFG_STR(optionName[APPROXIMANT], "SQT", CFGF_NONE),
		CFG_STR(optionName[SPIN], "ALL", CFGF_NONE),
		CFG_INT(optionName[PHASE], 4, CFGF_NONE),
		CFG_INT(optionName[AMPLITUDE], 2, CFGF_NONE),
		CFG_END()
	}, {
		CFG_SEC(optionName[BINARY], option.binary, CFGF_NONE),
		CFG_SEC(optionName[METHOD], option.method, CFGF_NONE),
		CFG_INT(optionName[NUMBER], 1, CFGF_NONE),
		CFG_STR(optionName[NAME], "wave", CFGF_NONE),
		CFG_END()
	}, {
		CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
		CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
		CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
		CFG_SEC(optionName[DEFAULT], option.defaultWave, CFGF_NONE),
		CFG_END()
	}
};

static int parseFrequency(cfg_t *config, Parameter *parameters);

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
	char *approximant, *spin;
	approximant = cfg_getstr(config, optionName[APPROXIMANT]);
	strcpy(method->approximant, approximant);
	spin = cfg_getstr(config, optionName[SPIN]);
	strcpy(method->spin, spin);
	method->phase = cfg_getint(config, optionName[PHASE]);
	method->amplitude = cfg_getint(config, optionName[AMPLITUDE]);
	return (SUCCESS);
}
static int parseBinary(cfg_t *config, Binary *binary) {
	cfg_t *spin;
	spin = cfg_getsec(config, optionName[SPIN]);
	parseSpin(spin, &binary->spin);
	for (int blackhole = MIN; blackhole < BH; blackhole++) {
		binary->mass[blackhole] = cfg_getnfloat(config, optionName[MASS], blackhole);
	}
	binary->inclination = cfg_getfloat(config, optionName[INCLINATION]);
	binary->distance = cfg_getfloat(config, optionName[DISTANCE]);
	return (SUCCESS);
}

static int parseWave(cfg_t *config, Wave *parameters) {
	cfg_t *binary = cfg_getsec(config, optionName[BINARY]);
	parseBinary(binary, &parameters->binary);
	cfg_t *generation = cfg_getsec(config, optionName[METHOD]);
	parseGeneration(generation, &parameters->method);
	char *name;
	name = cfg_getstr(config, optionName[NAME]);
	strcpy(parameters->name, name);
	parameters->number = cfg_getint(config, optionName[NUMBER]);
	return (SUCCESS);
}

int parse(char *file, Parameter *parameters) {
	cfg_t *config = cfg_init(option.option, CFGF_NONE);
	if (cfg_parse(config, file) == CFG_PARSE_ERROR) {
		return (FAILURE);
	}
	int success;
	success = parseFrequency(config, parameters);
	if (!success) {
		return (FAILURE);
	}
	cfg_t *defaultWave = cfg_getsec(config, optionName[DEFAULT]);
	success = parseWave(defaultWave, &parameters->defaultWave);
	if (!success) {
		return (FAILURE);
	}
	cfg_free(config);
	return (SUCCESS);
}

int printParameter(FILE *file, Parameter *parameter) {
	fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->initialFrequency, parameter->endingFrequency,
	        parameter->samplingFrequency);
	fprintf(file, "%11.5s %11.0u\n", parameter->defaultWave.name, parameter->defaultWave.number);
	fprintf(file, "%11.5s %11.5s % 11.0d % 11.0d\n", parameter->defaultWave.method.approximant,
	        parameter->defaultWave.method.spin, parameter->defaultWave.method.phase,
	        parameter->defaultWave.method.amplitude);
	fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g\n", parameter->defaultWave.binary.mass[0],
	        parameter->defaultWave.binary.mass[1], parameter->defaultWave.binary.inclination,
	        parameter->defaultWave.binary.distance);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->defaultWave.binary.spin.magnitude[blackhole],
		        parameter->defaultWave.binary.spin.inclination[blackhole],
		        parameter->defaultWave.binary.spin.azimuth[blackhole]);
	}
	return (SUCCESS);
}
