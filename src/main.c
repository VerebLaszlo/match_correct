/**	@file	main.c
 *	@author László Veréb
 *	@date	29.08.2012
 *	@brief	The main file.
 */

#include "stdio.h"
#include "stdlib.h"
#include "confuse.h"

/** Error codes. */
enum {
	FAILURE, SUCCESS,
};

/** Various constants. */
enum {
	BASE_OPTIONS = 4, NAME_LENGTH = 20,
};

/** IDs for the names of the options. */
enum {
	UNIT, ANGLE, MASS, DISTANCE, BOUNDARY_FREQUENCY, SAMPLING_FREQUENCY, OPTIONS,
};

/** Names of the options. */
char optionName[OPTIONS][NAME_LENGTH] =
        { "units", "angle", "mass", "distance", "boundaryFrequency", "samplingFrequency" };

/** Structure containing the options hierarchy. */
typedef struct {
	cfg_opt_t units[4];	    ///< Unit options.
	cfg_opt_t option[BASE_OPTIONS];	///< Group of the unit options.
} Option;

/** Option hierarchy. */
Option option = {
	{
		CFG_STR(optionName[ANGLE], "deg", CFGF_NONE),
		CFG_STR(optionName[MASS], "solar", CFGF_NONE),
		CFG_STR(optionName[DISTANCE], "Mpc", CFGF_NONE),
		CFG_END()
	}, {
		CFG_SEC(optionName[UNIT], option.units, CFGF_NONE),
		CFG_FLOAT_LIST(optionName[BOUNDARY_FREQUENCY], "{20.0, 2000.0}", CFGF_NONE),
		CFG_FLOAT(optionName[SAMPLING_FREQUENCY], 10240, CFGF_NONE),
		CFG_END()
	}
};

/**
 * Parse the config file.
 * @param[in] file name of the configuratin file
 * @return error code
 */
int parse(char *file) {
	cfg_t *cfg = cfg_init(option.option, CFGF_NONE);
	if (cfg_parse(cfg, file) == CFG_PARSE_ERROR) {
		return (FAILURE);
	}
	cfg_t *cfg1 = cfg_getsec(cfg, optionName[UNIT]);
	char *angle = cfg_getstr(cfg1, optionName[ANGLE]);
	char *mass = cfg_getstr(cfg1, optionName[MASS]);
	char *distance = cfg_getstr(cfg1, optionName[DISTANCE]);
	double boundaryFrequency[2];
	for (size_t i = 0; i < cfg_size(cfg, optionName[BOUNDARY_FREQUENCY]); i++) {
		boundaryFrequency[i] = cfg_getnfloat(cfg, optionName[BOUNDARY_FREQUENCY], i);
	}
	double samplingFrequency = cfg_getfloat(cfg, optionName[SAMPLING_FREQUENCY]);
	printf("angle   : %s\n", angle);
	printf("mass    : %s\n", mass);
	printf("distance: %s\n", distance);
	printf("freqI   : %g\n", boundaryFrequency[0]);
	printf("freqE   : %g\n", boundaryFrequency[1]);
	printf("freqS   : %g\n", samplingFrequency);
	cfg_free(cfg);
	return (SUCCESS);
}

/**
 * Main program function.
 * @param[in] argc number of arguments
 * @param[in] argv arguments
 * @return	error code
 */
int main(int argc, char *argv[]) {
	char *file = argc > 1 ? argv[1] : "test.conf";
	int success = parse(file);
	if (!success) {
		puts("Error!");
		exit(EXIT_FAILURE);
	}
	puts("OK!");
}
