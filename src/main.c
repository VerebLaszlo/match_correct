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

/** Unit options. */
cfg_opt_t units[] = { //
		CFG_STR("angle", "deg", CFGF_NONE), //
		CFG_STR("mass", "solar", CFGF_NONE), //
		CFG_STR("distance", "Mpc", CFGF_NONE), //
		CFG_END() };
/** Group of the unit options. */
cfg_opt_t unit[] = { CFG_SEC("units", units, CFGF_NONE), CFG_END() };

/**
 * Parse the config file.
 * @param[in] file name of the configuratin file
 * @return error code
 */
int parse(char *file) {
	cfg_t *cfg = cfg_init(unit, CFGF_NONE);
	if (cfg_parse(cfg, file) == CFG_PARSE_ERROR) {
		return (FAILURE);
	}
	cfg_t *cfg1 = cfg_getsec(cfg, "units");
	char *angle = cfg_getstr(cfg1, "angle");
	char *mass = cfg_getstr(cfg1, "mass");
	char *distance = cfg_getstr(cfg1, "distance");
	printf("angle   : %s\n", angle);
	printf("mass    : %s\n", mass);
	printf("distance: %s\n", distance);
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
