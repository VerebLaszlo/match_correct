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

/** Structure containing the output vectors. */
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

static int fillOutput(TimeSeries timeSeries[NUMBER_OF_WAVES], Output*output) {
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		for (int blackhole = 0; blackhole < BH; blackhole++) {
			memcpy(output->h[wave][blackhole], timeSeries[wave].h[blackhole]->data->data, output->length[wave]);
		}
		memcpy(output->V[wave], timeSeries->V->data->data, output->length[wave]);
		memcpy(output->Phi[wave], timeSeries[wave].Phi->data->data, output->length[wave]);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			memcpy(output->S1[wave][dimension], timeSeries[wave].S1[dimension]->data->data, output->length[wave]);
			memcpy(output->S2[wave][dimension], timeSeries[wave].S2[dimension]->data->data, output->length[wave]);
			memcpy(output->E1[wave][dimension], timeSeries[wave].E1[dimension]->data->data, output->length[wave]);
			memcpy(output->E3[wave][dimension], timeSeries[wave].E3[dimension]->data->data, output->length[wave]);
		}
	}
	return (SUCCESS);
}

/**
 * Creates outputs.
 * @param[in]  timeSeries generated time series.
 * @param[out] output     ?
 * @return failure code
 */
static int createOutput(TimeSeries timeSeries[NUMBER_OF_WAVES], Output *output) {
	int failure = SUCCESS;
	output->size =
	        timeSeries[FIRST].h[HP]->data->length > timeSeries[SECOND].h[HP]->data->length ?
	                timeSeries[FIRST].h[HP]->data->length : timeSeries[SECOND].h[HP]->data->length;
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		output->length[wave] = timeSeries[wave].h[HP]->data->length;
		size_t size = output->length[wave] * sizeof(double);
		output->length[wave] = timeSeries[wave].h[HP]->data->length;
		for (int blackhole = 0; blackhole < BH; blackhole++) {
			output->h[wave][blackhole] = malloc(size);
		}
		output->V[wave] = malloc(size);
		output->Phi[wave] = malloc(size);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			output->S1[wave][dimension] = malloc(size);
			output->S2[wave][dimension] = malloc(size);
			output->E1[wave][dimension] = malloc(size);
			output->E3[wave][dimension] = malloc(size);
		}
	}
	failure = fillOutput(timeSeries, output);
	return (failure);
}

/**
 * Cleans the structure.
 * @param[in] timeSeries memories to clean.
 */
static void cleanLAL(TimeSeries *timeSeries) {
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

int generateWaveformPair(Wave parameter[], double initialFrequency, double samplingTime, Output *output) {
	TimeSeries timeSeries[NUMBER_OF_WAVES];
	memset(timeSeries, 0, 2 * sizeof(TimeSeries));
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		generate(&parameter[wave], initialFrequency, samplingTime, &timeSeries[wave]);
	}
	createOutput(timeSeries, output);
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		cleanLAL(&timeSeries[wave]);
	}
}

void cleanOutput(Output *output) {
	for (int wave = FIRST; wave < NUMBER_OF_WAVES; wave++) {
		for (int blackhole = 0; blackhole < BH; blackhole++) {
			free(output->h[wave][blackhole]);
		}
		free(output->V[wave]);
		free(output->Phi[wave]);
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			free(output->S1[wave][dimension]);
			free(output->S2[wave][dimension]);
			free(output->E1[wave][dimension]);
			free(output->E3[wave][dimension]);
		}
	}
}

int printOutput(FILE *file, Output *output, Wave *wave, double samplingTime) {
	double M = wave->binary.mass[0] + wave->binary.mass[1];
	double eta = wave->binary.mass[0] * wave->binary.mass[1] / (M * M);
	fprintf(file, "#mass %11.5g %11.5g %11.5g %11.5g\n", wave->binary.mass[0], wave->binary.mass[1], M, eta);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		fprintf(file, "#spin %11.5g %11.5g %11.5g\n", wave->binary.spin.magnitude[blackhole],
		        degreeFromRadian(wave->binary.spin.inclination[blackhole]),
		        degreeFromRadian(wave->binary.spin.azimuth[blackhole]));
	}
	fprintf(file, "%11.5s %11.5s %11.5s %11.5s %11.5s ", "t", "phi1", "phi2", "omega1", "omega2");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s1x", "1s1y", "1s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2s1x", "2s1y", "2s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s2x", "1s2y", "1s2z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2s2x", "2s2y", "2s2z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e1x", "1e1y", "1e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2e1x", "2e1y", "2e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e3x", "1e3y", "1e3z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "2e3x", "2e3y", "2e3z");
	int shorter = output->length[FIRST] < output->length[SECOND] ? FIRST : SECOND;
	for (size_t index = 0; index < output->length[shorter]; index++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g ", index * samplingTime, output->Phi[FIRST][index],
		        output->Phi[SECOND][index], output->V[FIRST][index], output->V[SECOND][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S1[FIRST][X][index], output->S1[FIRST][Y][index],
		        output->S1[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S1[SECOND][X][index], output->S1[SECOND][Y][index],
		        output->S1[SECOND][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S2[FIRST][X][index], output->S2[FIRST][Y][index],
		        output->S2[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S2[SECOND][X][index], output->S2[SECOND][Y][index],
		        output->S2[SECOND][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->E1[FIRST][X][index], output->E1[FIRST][Y][index],
		        output->E1[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->E1[SECOND][X][index], output->E1[SECOND][Y][index],
		        output->E1[SECOND][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", output->E3[FIRST][X][index], output->E3[FIRST][Y][index],
		        output->E3[FIRST][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", output->E3[SECOND][X][index], output->E3[SECOND][Y][index],
		        output->E3[SECOND][Z][index]);
	}
	return (SUCCESS);
}
