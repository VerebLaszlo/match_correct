/**
 * @file binary_system.c
 *
 * @date Jul 20, 2011
 * @author vereb
 * @brief Binary system specific
 */

#include "test.h"
#include <assert.h>
#include "binary_system.h"

/// @name Generation functions
///@{

void generateBinarySystemParameters(BinarySystem *system, binaryLimits *limit) {
	BACKUP_DEFINITION_LINE(); //
	assert(system);
	assert(limit);
	system->inclination = randomBetween(limit->inclination[MIN], limit->inclination[MAX]);
	system->distance = randomBetween(limit->distance[MIN], limit->distance[MAX]);
	for (short i = 0; i < NUMBER_OF_BLACKHOLES; i++) {
		system->flatness[i] = randomBetween(limit->flatness[i][MIN], limit->flatness[i][MAX]);
	}
	generateMass(&system->mass, &limit->mass);
	for (short i = 0; i < NUMBER_OF_BLACKHOLES; i++) {
		generateSpin(&system->spin[i], &limit->spin[i], system->inclination);
	};
	SAVE_FUNCTION_FOR_TESTING();
}

///@}
/// @name Printing functions
///@{

void printBinarySystemParameters(FILE *file, BinarySystem *system, OutputFormat *format) {
	BACKUP_DEFINITION_LINE();
	printMassParameters(file, &system->mass, format);
	printSpinParameters(file, &system->spin[0], format);
	printSpinParameters(file, &system->spin[1], format);
	ushort number = 3;
	ushort length = (ushort) (number * format->widthWithSeparator);
	char formatString[length];
	setFormat(formatString, number, format);
	fprintf(file, formatString, system->flatness[0], system->flatness[1], system->inclination);
	setFormatEnd(formatString, number, format);
	fprintf(file, formatString, system->distance, system->coalescencePhase,
		system->coalescenceTime);
	SAVE_FUNCTION_FOR_TESTING();
}

///@}

#ifdef TEST
#include <math.h>
/// @name Testing functions
///@{

static bool isOK_generateBinarySystemParameters(void) {
	bool isOK = true;
	if (!areUtilMathFunctionsOK()) {
		isOK = false;
	} else if (!areBinarySystemMassFunctionsGood()) {
		isOK = false;
	} else if (!areBinarySystemSpinFunctionsGood()) {
		isOK = false;
	}
	BinarySystem system;
	binaryLimits limit;
	SAVE_FUNCTION_CALLER();
	limit.mass.mass[0][MAX] = 100.0 * (limit.mass.mass[0][MIN] = 1.0);
	limit.mass.mass[1][MAX] = 100.0 * (limit.mass.mass[1][MIN] = 1.0);
	limit.mass.totalMass[MAX] = 100.0 * (limit.mass.totalMass[MIN] = 2.0);
	limit.mass.eta[MAX] = 100.0 + (limit.mass.eta[MIN] = 0.0);
	limit.mass.chirpMass[MAX] = 100.0 + (limit.mass.chirpMass[MIN] = 0.0);
	limit.mass.mu[MAX] = 100.0 + (limit.mass.mu[MIN] = 0.0);
	limit.mass.nu[MAX] = 100.0 + (limit.mass.nu[MIN] = 0.0);
	limit.mass.m1_m2[MAX] = 100.0 + (limit.mass.m1_m2[MIN] = 0.0);
	for (ushort dim = 0; dim < DIMENSION; dim++) {
		limit.spin[0].component[FIXED][dim][MAX] = 100.0
			* (limit.spin[0].component[FIXED][dim][MIN] = 1.0);
		limit.spin[1].component[FIXED][dim][MAX] = 100.0
			* (limit.spin[1].component[FIXED][dim][MIN] = 1.0);
	}
	for (ushort i = 0; i < 2; i++) {
		limit.spin[i].magnitude[MAX] = 100.0 * (limit.spin[i].magnitude[MIN] = 1.0);
		limit.spin[i].azimuth[FIXED][MAX] = 10000.0
			* (limit.spin[i].azimuth[FIXED][MIN] = 0.000314);
		limit.spin[i].azimuth[PRECESSING][MAX] = 10000.0 * (limit.spin[i].azimuth[PRECESSING][MIN] =
			0.000314);
		limit.spin[i].inclination[FIXED][MAX] = 10000.0 * (limit.spin[i].inclination[FIXED][MIN] =
			0.000314);
		limit.spin[i].inclination[PRECESSING][MAX] = 10000.0
			* (limit.spin[i].inclination[PRECESSING][MIN] = 0.000314);
		limit.spin[i].elevation[FIXED][MAX] = M_PI
			+ (limit.spin[i].elevation[FIXED][MIN] = -M_PI_2);
		limit.spin[i].elevation[PRECESSING][MAX] = M_PI
			+ (limit.spin[i].elevation[PRECESSING][MIN] = -M_PI_4);
		limit.spin[i].component[FIXED][X][MAX] = 10.0 * (limit.spin[i].component[FIXED][X][MIN] =
			1.0);
		limit.spin[i].component[FIXED][Y][MAX] = 10.0 * (limit.spin[i].component[FIXED][Y][MIN] =
			1.0);
		limit.spin[i].component[FIXED][Z][MAX] = 10.0 * (limit.spin[i].component[FIXED][Z][MIN] =
			1.0);
		limit.spin[i].component[PRECESSING][X][MAX] = 40.0
			+ (limit.spin[i].component[PRECESSING][X][MIN] = -20.0);
		limit.spin[i].component[PRECESSING][Y][MAX] = 40.0
			+ (limit.spin[i].component[PRECESSING][Y][MIN] = -20.0);
		limit.spin[i].component[PRECESSING][Z][MAX] = 40.0
			+ (limit.spin[i].component[PRECESSING][Z][MIN] = -20.0);
	}
	limit.spin[0].mode = limit.spin[1].mode = GEN_FIXED_XYZ;
	generateBinarySystemParameters(&system, &limit);
	if (!isOK) {
		return isOK;
	}PRINT_OK();
	return isOK;
}

bool areBinarySystemFunctionsGood(void) {
	bool isOK = true;
	if (!isOK_generateBinarySystemParameters()) {
		isOK = false;
	}
	if (isOK) {
		PRINT_OK_FILE();
	} else {
		PRINT_ERROR_FILE();
	}
	return isOK;
}

///@}
#endif	// TEST
