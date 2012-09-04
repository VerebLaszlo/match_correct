/**	@file   parser_confuse.c
 *	@author László Veréb
 *	@date   29.08.2012
 *	@brief  Configuration file parser.
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
	int failure = SUCCESS;
	cfg_t *spin = cfg_getsec(config, optionName[SPIN]);
	for (int blackhole = MIN; blackhole < BH; blackhole++) {
		binary->mass[blackhole] = cfg_getnfloat(config, optionName[MASS], blackhole);
	}
	binary->inclination = cfg_getfloat(config, optionName[INCLINATION]);
	binary->distance = cfg_getfloat(config, optionName[DISTANCE]);
	failure = parseSpin(spin, &binary->spin);
	return (failure);
}

static int parseWave(cfg_t *config, Wave *parameters) {
	int failure = SUCCESS;
	char *name = cfg_getstr(config, optionName[NAME]);
	strcpy(parameters->name, name);
	parameters->number = cfg_getint(config, optionName[NUMBER]);
	cfg_t *binary = cfg_getsec(config, optionName[BINARY]);
	failure &= parseBinary(binary, &parameters->binary);
	cfg_t *generation = cfg_getsec(config, optionName[METHOD]);
	failure &= parseGeneration(generation, &parameters->method);
	return (failure);
}

void initParser(void) {
	memset(&defaultWave, 0, sizeof(Wave));
}

static int createList(double first, double second, char *string) {
	sprintf(string, "{%g, %g}", first, second);
	return (SUCCESS);
}

int parse(char *file, Parameter *parameters) {
	int failure = SUCCESS;
	char mass[STRING_LENGTH];
	failure &= createList(defaultWave.binary.mass[0], defaultWave.binary.mass[1], mass);
	char magnitude[STRING_LENGTH];
	failure &= createList(defaultWave.binary.spin.magnitude[0], defaultWave.binary.spin.magnitude[1], magnitude);
	char inclination[STRING_LENGTH];
	failure &= createList(defaultWave.binary.spin.inclination[0], defaultWave.binary.spin.inclination[1], inclination);
	char azimuth[STRING_LENGTH];
	failure &= createList(defaultWave.binary.spin.azimuth[0], defaultWave.binary.spin.azimuth[1], azimuth);
	if (!failure) {
		Option option = {	//
		        { CFG_STR(optionName[ANGLE], "deg", CFGF_NONE),
		        CFG_STR(optionName[MASS], "solar", CFGF_NONE),
		        CFG_STR(optionName[DISTANCE], "Mpc", CFGF_NONE),
		        CFG_END()
	        }, {
		        CFG_FLOAT_LIST(optionName[MAGNITUDE], magnitude, CFGF_NONE),
		        CFG_FLOAT_LIST(optionName[INCLINATION], inclination, CFGF_NONE),
		        CFG_FLOAT_LIST(optionName[AZIMUTH], azimuth, CFGF_NONE),
		        CFG_STR(optionName[COORDINATE_SYSTEM], "precessing", CFGF_NONE),
		        CFG_END()
	        }, {
		        CFG_FLOAT_LIST(optionName[MASS], mass, CFGF_NONE),
		        CFG_SEC(optionName[SPIN], option.spin, CFGF_NONE),
		        CFG_FLOAT(optionName[INCLINATION], defaultWave.binary.inclination, CFGF_NONE),
		        CFG_FLOAT(optionName[DISTANCE], defaultWave.binary.distance, CFGF_NONE),
		        CFG_END()
	        }, {
		        CFG_STR(optionName[APPROXIMANT], defaultWave.method.approximant, CFGF_NONE),
		        CFG_STR(optionName[SPIN], defaultWave.method.spin, CFGF_NONE),
		        CFG_INT(optionName[PHASE], defaultWave.method.phase, CFGF_NONE),
		        CFG_INT(optionName[AMPLITUDE], defaultWave.method.amplitude, CFGF_NONE),
		        CFG_END()
	        }, {
		        CFG_SEC(optionName[BINARY], option.binary, CFGF_NONE),
		        CFG_SEC(optionName[METHOD], option.method, CFGF_NONE),
		        CFG_INT(optionName[NUMBER], defaultWave.number, CFGF_NONE),
		        CFG_STR(optionName[NAME], defaultWave.name, CFGF_NONE),
		        CFG_END()
	        }, {
		        CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
		        CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
		        CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
		        CFG_SEC(optionName[DEFAULT], option.defaultWave, CFGF_NONE),
		        CFG_END()
	        }
        };
        cfg_t *config = cfg_init(option.option, CFGF_NONE);
        failure = cfg_parse(config, file) == CFG_PARSE_ERROR;
        if (!failure) {
	        failure = parseFrequency(config, parameters);
	        cfg_t *wave = cfg_getsec(config, optionName[DEFAULT]);
	        failure &= parseWave(wave, &defaultWave);
        }
        cfg_free(config);
    }
	memcpy(&parameters->wave, &defaultWave, sizeof(Wave));
	return (failure);
}

int printParameter(FILE *file, Parameter *parameter) {
	fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->initialFrequency, parameter->endingFrequency,
	        parameter->samplingFrequency);
	fprintf(file, "%11.5s %11.0u\n", parameter->wave.name, parameter->wave.number);
	fprintf(file, "%11.5s %11.5s % 11.0d % 11.0d\n", parameter->wave.method.approximant, parameter->wave.method.spin,
	        parameter->wave.method.phase, parameter->wave.method.amplitude);
	fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g\n", parameter->wave.binary.mass[0], parameter->wave.binary.mass[1],
	        parameter->wave.binary.inclination, parameter->wave.binary.distance);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->wave.binary.spin.magnitude[blackhole],
		        parameter->wave.binary.spin.inclination[blackhole], parameter->wave.binary.spin.azimuth[blackhole]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", parameter->wave.binary.spin.component[blackhole][X],
		        parameter->wave.binary.spin.component[blackhole][Y],
		        parameter->wave.binary.spin.component[blackhole][Z]);
	}
	return (SUCCESS);
}
