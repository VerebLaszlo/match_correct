/**	@file	generator_lal.c
 *	@author László Veréb
 *	@date	3.09.2012
 *	@brief	Waveform generator.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <lal/LALConstants.h>
#include <lal/LALDatatypes.h>
#include <lal/LALSimInspiral.h>
#include <lal/TimeSeries.h>
#include "generator_lal.h"

/** Various constants. */
enum {
	MEGA = 1000000,
};

/**
 * Gets the interaction from strig.
 * @param[in] interaction interaction string.
 * @return interaction code.
 */
static LALSimInspiralInteraction getInteraction(const char * const interaction) {
	LALSimInspiralInteraction spin = LAL_SIM_INSPIRAL_INTERACTION_NONE;
	if (!strstr(interaction, "NO")) {
		if (strstr(interaction, "ALL")) {
			spin = LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN | LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN
			        | LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN | LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN;
		} else {
			if (strstr(interaction, "SO")) {
				spin |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN;
			}
			if (strstr(interaction, "SS")) {
				spin |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN;
			}
			if (strstr(interaction, "SE")) {
				spin |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN;
			}
			if (strstr(interaction, "QM")) {
				spin |= LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN;
			}
		}
	}
	return (spin);
}

/**
 * Converts spin from precessing angle representation to fixed component representation.
 * @param[in,out] spin    angles, magnitude, and components
 * @param[in] inclination angle of the precessing z axis.
 * @return failure code
 */
static int convertSpinFromAnglesToXyz(Spin *spin, double inclination) {
	double theta[2] = { -0.0, inclination };
	double cosAzimuth, cosInclination;
	double component[BH][DIMENSION];
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		cosAzimuth = cosGood(spin->azimuth[blackhole]);
		cosInclination = cosGood(spin->inclination[blackhole]);
		component[blackhole][X] = spin->magnitude[blackhole] * sinGood(spin->inclination[blackhole]) * cosAzimuth;
		component[blackhole][Y] = spin->magnitude[blackhole] * sinGood(spin->inclination[blackhole])
		        * sinGood(spin->azimuth[blackhole]);
		component[blackhole][Z] = spin->magnitude[blackhole] * cosInclination;
		spin->component[blackhole][X] = +component[blackhole][X] * cosGood(theta[0]) * cosGood(theta[1])
		        + component[blackhole][Y] * sinGood(theta[0])
		        - component[blackhole][Z] * cosGood(theta[0]) * sinGood(theta[1]);
		spin->component[blackhole][Y] = -component[blackhole][X] * sinGood(theta[0]) * sinGood(theta[1])
		        + component[blackhole][Y] * cosGood(theta[0])
		        + component[blackhole][Z] * sinGood(theta[0]) * sinGood(theta[1]);
		spin->component[blackhole][Z] = +component[blackhole][X] * sinGood(theta[1])
		        + component[blackhole][Z] * cosGood(theta[1]);
	}
	return (SUCCESS);
}

/** Structure containing the variable vectors. */
typedef struct {
	REAL8TimeSeries *h[WAVE];
	REAL8TimeSeries *V;
	REAL8TimeSeries *Phi;
	REAL8TimeSeries *S1[DIMENSION];
	REAL8TimeSeries *S2[DIMENSION];
	REAL8TimeSeries *E1[DIMENSION];
	REAL8TimeSeries *E3[DIMENSION];
} TimeSeries;

/**
 * Cleans the structure.
 * @param[in] timeSeries memories to clean.
 */
static void destroyTimeSeries(TimeSeries *timeSeries) {
	XLALDestroyREAL8TimeSeries(timeSeries->h[HP]);
	XLALDestroyREAL8TimeSeries(timeSeries->h[HC]);
	XLALDestroyREAL8TimeSeries(timeSeries->V);
	XLALDestroyREAL8TimeSeries(timeSeries->Phi);
	for (int dimension = X; dimension < DIMENSION; dimension++) {
		XLALDestroyREAL8TimeSeries(timeSeries->S1[dimension]);
		XLALDestroyREAL8TimeSeries(timeSeries->S2[dimension]);
		XLALDestroyREAL8TimeSeries(timeSeries->E1[dimension]);
		XLALDestroyREAL8TimeSeries(timeSeries->E3[dimension]);
	}
}

/**
 * Creates outputs.
 * @param[in]  timeSeries generated time series.
 * @param[out] variable     ?
 * @return failure code
 */
static Variable *createOutput(TimeSeries timeSeries[NUMBER_OF_WAVES]) {
	Variable *variable = calloc(1, sizeof(Variable));
	variable->size =
	        timeSeries[FIRST].h[HP]->data->length > timeSeries[SECOND].h[HC]->data->length ?
	                timeSeries[FIRST].h[HP]->data->length : timeSeries[SECOND].h[HC]->data->length;
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		variable->length[wave] = timeSeries[wave].h[HP]->data->length;
		size_t size = variable->length[wave] * sizeof(double);
		variable->V[wave] = malloc(size);
		variable->Phi[wave] = malloc(size);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			variable->S1[wave][dimension] = malloc(size);
			variable->S2[wave][dimension] = malloc(size);
			variable->E1[wave][dimension] = malloc(size);
			variable->E3[wave][dimension] = malloc(size);
		}
	}
	variable->wave = createWaveform(variable->length[FIRST], variable->length[SECOND]);
	return (variable);
}

void destroyOutput(Variable **variable) {
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		free((*variable)->V[wave]);
		free((*variable)->Phi[wave]);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			free((*variable)->S1[wave][dimension]);
			free((*variable)->S2[wave][dimension]);
			free((*variable)->E1[wave][dimension]);
			free((*variable)->E3[wave][dimension]);
		}
	}
	free(*variable);
}

static int fillOutput(TimeSeries timeSeries[NUMBER_OF_WAVES], Variable*variable) {
	size_t size = sizeof(double);
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		for (int component = 0; component < WAVE; component++) {
			memcpy(variable->wave->h[2 * wave + component], timeSeries[wave].h[component]->data->data,
			        variable->length[wave] * size);
		}
		memcpy(variable->V[wave], timeSeries->V->data->data, variable->length[wave] * size);
		memcpy(variable->Phi[wave], timeSeries[wave].Phi->data->data, variable->length[wave] * size);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			memcpy(variable->S1[wave][dimension], timeSeries[wave].S1[dimension]->data->data,
			        variable->length[wave] * size);
			memcpy(variable->S2[wave][dimension], timeSeries[wave].S2[dimension]->data->data,
			        variable->length[wave] * size);
			memcpy(variable->E1[wave][dimension], timeSeries[wave].E1[dimension]->data->data,
			        variable->length[wave] * size);
			memcpy(variable->E3[wave][dimension], timeSeries[wave].E3[dimension]->data->data,
			        variable->length[wave] * size);
		}
	}
	return (SUCCESS);
}

static int generate(Wave *wave, double initialFrequency, double samplingTime, TimeSeries *timeSeries) {
	int failure = SUCCESS;
	convertSpinFromAnglesToXyz(&wave->binary.spin, wave->binary.inclination);
	REAL8 e1[DIMENSION] = { +cos(wave->binary.inclination), 0.0, -sin(wave->binary.inclination) };
	REAL8 e3[DIMENSION] = { +sin(wave->binary.inclination), 0.0, +cos(wave->binary.inclination) };
	LALSimInspiralInteraction interactionFlags = getInteraction(wave->method.spin);
	failure = XLALSimInspiralSpinQuadTaylorEvolveAll(&timeSeries->h[HP], &timeSeries->h[HC], &timeSeries->V,
	        &timeSeries->Phi, &timeSeries->S1[X], &timeSeries->S1[Y], &timeSeries->S1[Z], &timeSeries->S2[X],
	        &timeSeries->S2[Y], &timeSeries->S2[Z], &timeSeries->E3[X], &timeSeries->E3[Y], &timeSeries->E3[Z],
	        &timeSeries->E1[X], &timeSeries->E1[Y], &timeSeries->E1[Z], wave->binary.mass[0] * LAL_MSUN_SI,
	        wave->binary.mass[1] * LAL_MSUN_SI, 1.0, 1.0, wave->binary.spin.component[0][X],
	        wave->binary.spin.component[0][Y], wave->binary.spin.component[0][Z], wave->binary.spin.component[1][X],
	        wave->binary.spin.component[1][Y], wave->binary.spin.component[1][Z], e3[X], e3[Y], e3[Z], e1[X], e1[Y],
	        e1[Z], wave->binary.distance * MEGA * LAL_PC_SI, 0.0, initialFrequency, 0.0, samplingTime,
	        wave->method.phase, wave->method.amplitude, interactionFlags);
	return (failure);
}

Variable* generateWaveformPair(Wave parameter[], double initialFrequency, double samplingTime) {
	TimeSeries timeSeries[NUMBER_OF_WAVES];
	memset(timeSeries, 0, 2 * sizeof(TimeSeries));
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		generate(&parameter[wave], initialFrequency, samplingTime, &timeSeries[wave]);
	}
	Variable *variable = createOutput(timeSeries);
	fillOutput(timeSeries, variable);
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		destroyTimeSeries(&timeSeries[wave]);
	}
	return (variable);
}

static void printHeader(FILE *file, Wave *wave) {
	double M = wave->binary.mass[0] + wave->binary.mass[1];
	double eta = wave->binary.mass[0] * wave->binary.mass[1] / (M * M);
	fprintf(file, "#mass %11.5g %11.5g %11.5g %11.5g\n", wave->binary.mass[0], wave->binary.mass[1], M, eta);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		fprintf(file, "#spin %11.5g %11.5g %11.5g\n", wave->binary.spin.magnitude[blackhole],
		        degreeFromRadian(wave->binary.spin.inclination[blackhole]),
		        degreeFromRadian(wave->binary.spin.azimuth[blackhole]));
	}
}

void printSpins(FILE *file, Variable *variable, Wave *wave, double samplingTime) {
	printHeader(file, wave);
	fprintf(file, "%11.5s ", "t");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s1x", "1s1y", "1s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2s1x", "2s1y", "2s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s2x", "1s2y", "1s2z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "2s2x", "2s2y", "2s2z");
	int shorter = variable->length[FIRST] < variable->length[SECOND] ? FIRST : SECOND;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g ", index * samplingTime);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S1[FIRST][X][index], variable->S1[FIRST][Y][index],
		        variable->S1[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S1[SECOND][X][index], variable->S1[SECOND][Y][index],
		        variable->S1[SECOND][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S2[FIRST][X][index], variable->S2[FIRST][Y][index],
		        variable->S2[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", variable->S2[SECOND][X][index], variable->S2[SECOND][Y][index],
		        variable->S2[SECOND][Z][index]);
	}
}

void printSystem(FILE *file, Variable *variable, Wave *wave, double samplingTime) {
	printHeader(file, wave);
	fprintf(file, "%11.5s ", "t");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e1x", "1e1y", "1e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2e1x", "2e1y", "2e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e3x", "1e3y", "1e3z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "2e3x", "2e3y", "2e3z");
	int shorter = variable->length[FIRST] < variable->length[SECOND] ? FIRST : SECOND;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g ", index * samplingTime);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E1[FIRST][X][index], variable->E1[FIRST][Y][index],
		        variable->E1[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E1[SECOND][X][index], variable->E1[SECOND][Y][index],
		        variable->E1[SECOND][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E3[FIRST][X][index], variable->E3[FIRST][Y][index],
		        variable->E3[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", variable->E3[SECOND][X][index], variable->E3[SECOND][Y][index],
		        variable->E3[SECOND][Z][index]);
	}
}

int printOutput(FILE *file, Variable *variable, Wave *wave, double samplingTime) {
	printHeader(file, wave);
	fprintf(file, "%11.5s %11.5s %11.5s %11.5s %11.5s %11.5s %11.5s %11.5s %11.5s %11.5s %11.5s\n", "t", "h1", "h2",
	        "hp1", "hc1", "hp2", "hc2", "phi1", "phi2", "omega1", "omega2");
	int shorter = variable->length[FIRST] < variable->length[SECOND] ? FIRST : SECOND;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g ", index * samplingTime,
		        variable->wave->H[HP][index], variable->wave->H[HC][index], variable->wave->h[HP1][index],
		        variable->wave->h[HC1][index], variable->wave->h[HP2][index], variable->wave->h[HC2][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g\n", variable->Phi[FIRST][index], variable->Phi[SECOND][index],
		        variable->V[FIRST][index], variable->V[SECOND][index]);
	}
	return (SUCCESS);
}
