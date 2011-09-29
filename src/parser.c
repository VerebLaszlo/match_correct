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
	NUMBER_OF_OPTIONS,
} OptionCode;

char const * optionName[] = { "boundaryFrequency", "samplingFrequency", "default", "source",
								"binary", "masses", "spins", "magnitude", "inclination", "azimuth",
								"distance", "detector", "generation", "approximant", "phase",
								"spin", "amplitude", "name" };

static void getLimits(config_setting_t *limits, double *min, double *max) {
	if (config_setting_length(limits) != 2) {
		exit(EXIT_FAILURE);
	}
	*min = config_setting_get_float_elem(limits, MIN);
	*max = config_setting_get_float_elem(limits, MAX);
}

static void getMasses(config_setting_t *binary, SystemParameter parameter[]) {
	config_setting_t *masses = config_setting_get_member(binary, optionName[MASSES]);
	int count = config_setting_length(masses);
	for (ushort i = 0; i < count; i++) {
		config_setting_t *mass = config_setting_get_elem(masses, i);
		getLimits(mass, &parameter[MIN].system[0].mass.mass[i],
			&parameter[MAX].system[0].mass.mass[i]);
	}
}

static void getSpins(config_setting_t *binary, SystemParameter parameter[]) {
	config_setting_t *current;
	config_setting_t *spins = config_setting_get_member(binary, optionName[SPINS]);
	int count = config_setting_length(spins);
	for (ushort i = 0; i < count; i++) {
		config_setting_t *spin = config_setting_get_elem(spins, i);
		current = config_setting_get_member(spin, optionName[MAGNITUDE]);
		getLimits(current, &parameter[MIN].system[0].spin[i].magnitude,
			&parameter[MAX].system[0].spin[i].magnitude);
		current = config_setting_get_member(spin, optionName[INCLINATION]);
		getLimits(current, &parameter[MIN].system[0].spin[i].inclination[PRECESSING],
			&parameter[MAX].system[0].spin[i].inclination[PRECESSING]);
		current = config_setting_get_member(spin, optionName[AZIMUTH]);
		getLimits(current, &parameter[MIN].system[0].spin[i].azimuth[PRECESSING],
			&parameter[MAX].system[0].spin[i].azimuth[PRECESSING]);
	}
}

static void getSourceParameters(config_setting_t *waveform, SystemParameter *parameter) {
	config_setting_t *source = config_setting_get_member(waveform, optionName[SOURCE]);
	getMasses(source, parameter);
	getSpins(source, parameter);
	config_setting_t *current;
	current = config_setting_get_member(source, optionName[INCLINATION]);
	getLimits(current, &parameter[MIN].system[0].inclination,
		&parameter[MAX].system[0].inclination);
	current = config_setting_get_member(source, optionName[DISTANCE]);
	getLimits(current, &parameter[MIN].system[0].distance, &parameter[MAX].system[0].distance);
}

typedef const char *cstring;
static void getGenerationParameters(config_setting_t *waveform, SystemParameter *parameter) {
	config_setting_t *generation = config_setting_get_member(waveform, optionName[GENERATION]);
	cstring approximant, phase, spin, amplitude;
	config_setting_lookup_string(generation, optionName[APPROXIMANT], &approximant);
	strcpy(parameter[0].approximant[0], approximant);
	config_setting_lookup_string(generation, optionName[PHASE], &phase);
	strcpy(parameter[0].phase[0], phase);
	config_setting_lookup_string(generation, optionName[SPIN], &spin);
	strcpy(parameter[0].spin[0], spin);
	config_setting_lookup_string(generation, optionName[AMPLITUDE], &amplitude);
	strcpy(parameter[0].amplitude[0], amplitude);
}

static void getWaveformParameters(config_setting_t *waveform, SystemParameter *parameter) {
	getSourceParameters(waveform, parameter);
	getGenerationParameters(waveform, parameter);
	cstring name;
	config_setting_lookup_string(waveform, optionName[NAME], &name);
	strcpy(parameter[0].name[0], name);
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
	getWaveformParameters(defaultWave, parameter);
	config_destroy(&cfg);
}
