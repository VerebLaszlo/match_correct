/**	@file	generator_lal.c
 *	@author László Veréb
 *	@date	3.09.2012
 *	@brief	Waveform generator.
 */

#include <math.h>
#include <string.h>
#include "generator_lal.h"
#include "util_IO.h"

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
 * @return success code
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

/**
 * Frees the allocated memory.
 * @param[in] output memories to free.
 * @return success code
 */
static int cleanLAL(Output *output) {
	XLALDestroyREAL8TimeSeries(output->h[HP]);
	XLALDestroyREAL8TimeSeries(output->h[HC]);
	XLALDestroyREAL8TimeSeries(output->V);
	XLALDestroyREAL8TimeSeries(output->Phi);
	for (int dimension = X; dimension < DIMENSION; dimension++) {
		XLALDestroyREAL8TimeSeries(output->S1[dimension]);
		XLALDestroyREAL8TimeSeries(output->S2[dimension]);
		XLALDestroyREAL8TimeSeries(output->E1[dimension]);
		XLALDestroyREAL8TimeSeries(output->E3[dimension]);
	}
	return (SUCCESS);
}

/**
 * Prints the generated values to a file.
 * @param[in] file   where to print.
 * @param[in] output what to print.
 * @param[in] dt     sampling time.
 * @return success code
 */
int printOutput(FILE *file, Output *output, double dt) {
	double sqrt2p2 = M_SQRT2 / 2.0;
	fprintf(file, "%11.5s %11.5s %11.5s %11.5s %11.5s %11.5s ", "t", "h", "hp", "hc", "phi", "omega");
	fprintf(file, "%11.5s %11.5s %11.5s ", "s1x", "s1y", "s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "s2x", "s2y", "s2z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "e1x", "e1y", "e1z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "e3x", "e3y", "e3z");
	for (int index = 0; index < 10/*output->h[HP]->data->length*/; index++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g ", index * dt,
		        sqrt2p2 * (output->h[HP]->data->data[index] + output->h[HC]->data->data[index]),
		        output->h[HP]->data->data[index], output->h[HC]->data->data[index], output->Phi->data->data[index],
		        output->V->data->data[index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S1[X]->data->data[index], output->S1[Y]->data->data[index],
		        output->S1[Z]->data->data[index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->S2[X]->data->data[index], output->S2[Y]->data->data[index],
		        output->S2[Z]->data->data[index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", output->E1[X]->data->data[index], output->E1[Y]->data->data[index],
		        output->E1[Z]->data->data[index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", output->E3[X]->data->data[index], output->E3[Y]->data->data[index],
		        output->E3[Z]->data->data[index]);
	}
	return (SUCCESS);
}

/**
 * Generates a waveform.
 * @param[in] parameter defining the waveform.
 * @return success code
 */
int generate(Parameter *parameter) {
	convertSpinFromAnglesToXyz(&parameter->wave.binary.spin, parameter->wave.binary.inclination);
	Output output;
	memset(&output, 0, sizeof(Output));
	REAL8 e1[DIMENSION] = { +cos(parameter->wave.binary.inclination), 0.0, -sin(parameter->wave.binary.inclination) };
	REAL8 e3[DIMENSION] = { +sin(parameter->wave.binary.inclination), 0.0, +cos(parameter->wave.binary.inclination) };
	LALSimInspiralInteraction interactionFlags = getInteraction(parameter->defaultWave.method.spin);
	int error = XLALSimInspiralSpinQuadTaylorEvolveAll(&output.h[HP], &output.h[HC], &output.V, &output.Phi,
	        &output.S1[X], &output.S1[Y], &output.S1[Z], &output.S2[X], &output.S2[Y], &output.S2[Z], &output.E3[X],
	        &output.E3[Y], &output.E3[Z], &output.E1[X], &output.E1[Y], &output.E1[Z],
	        parameter->wave.binary.mass[0] * LAL_MSUN_SI, parameter->wave.binary.mass[1] * LAL_MSUN_SI, 1.0, 1.0,
	        parameter->wave.binary.spin.component[0][X], parameter->wave.binary.spin.component[0][Y],
	        parameter->wave.binary.spin.component[0][Z], parameter->wave.binary.spin.component[1][X],
	        parameter->wave.binary.spin.component[1][Y], parameter->wave.binary.spin.component[1][Z], e3[X], e3[Y],
	        e3[Z], e1[X], e1[Y], e1[Z], parameter->wave.binary.distance * MEGA * LAL_PC_SI, 0.0,
	        parameter->initialFrequency, 0.0, parameter->samplingTime, parameter->wave.method.phase,
	        parameter->wave.method.amplitude, interactionFlags);
	if (error) {
		printf("Error: %d\n", error);
		return (FAILURE);
	}
	FILE *file = safelyOpenForWriting("out/all.txt");
	printOutput(file, &output, parameter->samplingTime);
	fclose(file);
	cleanLAL(&output);
	return (SUCCESS);
}
