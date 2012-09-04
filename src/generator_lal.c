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
		        + component[blackhole][Y] * cosGood(theta[1]);
	}
	return (SUCCESS);
}

static int fillOutput(TimeSeries *timeSeries, Output*output) {
	for (size_t index = 0; index < output->length; index++) {
		for (int blackhole = 0; blackhole < BH; blackhole++) {
			output->h[blackhole][index] = timeSeries->h[blackhole]->data->data[index];
		}
		output->V[index] = timeSeries->V->data->data[index];
		output->Phi[index] = timeSeries->Phi->data->data[index];
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			output->S1[dimension][index] = timeSeries->S1[dimension]->data->data[index];
			output->S2[dimension][index] = timeSeries->S2[dimension]->data->data[index];
			output->E1[dimension][index] = timeSeries->E1[dimension]->data->data[index];
			output->E3[dimension][index] = timeSeries->E3[dimension]->data->data[index];
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
static int createOutput(TimeSeries *timeSeries, Output *output) {
	int failure = SUCCESS;
	output->length = timeSeries->h[HP]->data->length;
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		output->h[blackhole] = malloc(output->length * sizeof(double));
	}
	output->V = malloc(output->length * sizeof(double));
	output->Phi = malloc(output->length * sizeof(double));
	for (int dimension = X; dimension < DIMENSION; dimension++) {
		output->S1[dimension] = malloc(output->length * sizeof(double));
		output->S2[dimension] = malloc(output->length * sizeof(double));
		output->E1[dimension] = malloc(output->length * sizeof(double));
		output->E3[dimension] = malloc(output->length * sizeof(double));
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

int generate(Parameter *parameter, Output *output) {
	int failure = SUCCESS;
	convertSpinFromAnglesToXyz(&parameter->wave.binary.spin, parameter->wave.binary.inclination);
	TimeSeries timeSeries;
	memset(&timeSeries, 0, sizeof(TimeSeries));
	REAL8 e1[DIMENSION] = { +cos(parameter->wave.binary.inclination), 0.0, -sin(parameter->wave.binary.inclination) };
	REAL8 e3[DIMENSION] = { +sin(parameter->wave.binary.inclination), 0.0, +cos(parameter->wave.binary.inclination) };
	LALSimInspiralInteraction interactionFlags = getInteraction(parameter->wave.method.spin);
	failure = XLALSimInspiralSpinQuadTaylorEvolveAll(&timeSeries.h[HP], &timeSeries.h[HC], &timeSeries.V,
	        &timeSeries.Phi, &timeSeries.S1[X], &timeSeries.S1[Y], &timeSeries.S1[Z], &timeSeries.S2[X],
	        &timeSeries.S2[Y], &timeSeries.S2[Z], &timeSeries.E3[X], &timeSeries.E3[Y], &timeSeries.E3[Z],
	        &timeSeries.E1[X], &timeSeries.E1[Y], &timeSeries.E1[Z], parameter->wave.binary.mass[0] * LAL_MSUN_SI,
	        parameter->wave.binary.mass[1] * LAL_MSUN_SI, 1.0, 1.0, parameter->wave.binary.spin.component[0][X],
	        parameter->wave.binary.spin.component[0][Y], parameter->wave.binary.spin.component[0][Z],
	        parameter->wave.binary.spin.component[1][X], parameter->wave.binary.spin.component[1][Y],
	        parameter->wave.binary.spin.component[1][Z], e3[X], e3[Y], e3[Z], e1[X], e1[Y], e1[Z],
	        parameter->wave.binary.distance * MEGA * LAL_PC_SI, 0.0, parameter->initialFrequency, 0.0,
	        parameter->samplingTime, parameter->wave.method.phase, parameter->wave.method.amplitude, interactionFlags);
	if (!failure) {
		failure = createOutput(&timeSeries, output);
	}
	cleanLAL(&timeSeries);
	return (failure);
}

void cleanOutput(Output *output) {
	output->length = 0;
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		free(output->h[blackhole]);
	}
	free(output->V);
	free(output->Phi);
	for (int dimension = X; dimension < DIMENSION; dimension++) {
		free(output->S1[dimension]);
		free(output->S2[dimension]);
		free(output->E1[dimension]);
		free(output->E3[dimension]);
	}
}

int printOutput(FILE *file, Output *output, Parameter *parameter) {
	double sqrt2p2 = M_SQRT2 / 2.0;
	double M = parameter->wave.binary.mass[0] + parameter->wave.binary.mass[1];
	double eta = parameter->wave.binary.mass[0] * parameter->wave.binary.mass[1] / (M * M);
	fprintf(file, "#mass %11.5g %11.5g %11.5g %11.5g\n", parameter->wave.binary.mass[0], parameter->wave.binary.mass[1],
	        M, eta);
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		fprintf(file, "#spin %11.5g %11.5g %11.5g\n", parameter->wave.binary.spin.magnitude[blackhole],
		        degreeFromRadian(parameter->wave.binary.spin.inclination[blackhole]),
		        degreeFromRadian(parameter->wave.binary.spin.azimuth[blackhole]));
	}
	fprintf(file, "%11.5s %11.5s %11.5s %11.5s %11.5s %11.5s ", "t", "h", "hp", "hc", "phi", "omega");
	fprintf(file, "%11.5s %11.5s %11.5s ", "s1x", "s1y", "s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "s2x", "s2y", "s2z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "e1x", "e1y", "e1z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "e3x", "e3y", "e3z");
	for (size_t index = 0; index < output->length; index++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g ", index * parameter->samplingTime,
		        sqrt2p2 * (output->h[HP][index] + output->h[HC][index]), output->h[HP][index], output->h[HC][index],
		        output->Phi[index], output->V[index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S1[X][index], output->S1[Y][index], output->S1[Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S2[X][index], output->S2[Y][index], output->S2[Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->E1[X][index], output->E1[Y][index], output->E1[Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", output->E3[X][index], output->E3[Y][index], output->E3[Z][index]);
	}
	return (SUCCESS);
}
