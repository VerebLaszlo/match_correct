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
	HP, HC, WAVE, MEGA = 1000000,
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

/*
 static void getStringInteraction(LALSimInspiralInteraction interaction, char *spin) {
 string text = "";
 strcpy(spin, "");
 switch (interaction) {
 case LAL_SIM_INSPIRAL_INTERACTION_NONE:
 strcpy(spin, "NO");
 break;
 case LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN | LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN
 | LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN | LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN:
 strcpy(spin, "ALL");
 break;
 case LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN:
 strcpy(text, "SO");
 strcpy(spin, text);
 // no break
 case LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN:
 sprintf(text, "%s%s", spin, "SS");
 strcpy(spin, text);
 // no break
 case LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN:
 sprintf(text, "%s%s", spin, "SE");
 strcpy(spin, text);
 // no break
 case LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN:
 sprintf(text, "%s%s", spin, "QM");
 strcpy(spin, text);
 break;
 default:
 break;
 }
 }
 */
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
static Variable *createOutput(TimeSeries timeSeries[NUMBER_OF_WAVE]) {
	Variable *variable = calloc(1, sizeof(Variable));
	variable->size =
	        timeSeries[FIRST_WAVE].h[HP]->data->length > timeSeries[SECOND_WAVE].h[HC]->data->length ?
	                timeSeries[FIRST_WAVE].h[HP]->data->length : timeSeries[SECOND_WAVE].h[HC]->data->length;
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		variable->length[wave] = timeSeries[wave].h[HP]->data->length;
		variable->V[wave] = calloc(variable->length[wave], sizeof(double));
		variable->Phi[wave] = calloc(variable->length[wave], sizeof(double));
		for (int dimension = X; dimension < DIMENSION; dimension++) {
			variable->S1[wave][dimension] = calloc(variable->length[wave], sizeof(double));
			variable->S2[wave][dimension] = calloc(variable->length[wave], sizeof(double));
			variable->E1[wave][dimension] = calloc(variable->length[wave], sizeof(double));
			variable->E3[wave][dimension] = calloc(variable->length[wave], sizeof(double));
		}
	}
	variable->wave = createWaveform(variable->length[FIRST_WAVE], variable->length[SECOND_WAVE]);
	return (variable);
}

void destroyOutput(Variable **variable) {
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
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

static int fillOutput(TimeSeries timeSeries[NUMBER_OF_WAVE], Variable*variable) {
	size_t size = sizeof(double);
	double sqt2_2 = M_SQRT2 / 2.0;
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		for (int component = HP; component < WAVE; component++) {
			memcpy(variable->wave->h[2 * wave + component], timeSeries[wave].h[component]->data->data,
			        variable->length[wave] * size);
		}
		for (size_t index = 0; index < variable->wave->length[wave]; index++) {
			variable->wave->H[wave][index] = sqt2_2
			        * (variable->wave->h[2 * wave][index] + variable->wave->h[2 * wave + 1][index]);
		}
		memcpy(variable->V[wave], timeSeries[wave].V->data->data, variable->length[wave] * size);
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
	TimeSeries timeSeries[NUMBER_OF_WAVE];
	memset(timeSeries, 0, 2 * sizeof(TimeSeries));
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		generate(&parameter[wave], initialFrequency, samplingTime, &timeSeries[wave]);
	}
	Variable *variable = createOutput(timeSeries);
	fillOutput(timeSeries, variable);
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		destroyTimeSeries(&timeSeries[wave]);
	}
	return (variable);
}

static void printHeader(FILE *file, Wave parameter[2], Analysed *analysed) {
	double M[NUMBER_OF_WAVE] = {
	    parameter[FIRST_WAVE].binary.mass[0] + parameter[FIRST_WAVE].binary.mass[1],
	    parameter[SECOND_WAVE].binary.mass[0] + parameter[SECOND_WAVE].binary.mass[1] };
	double eta[NUMBER_OF_WAVE] = { parameter[FIRST_WAVE].binary.mass[0] * parameter[FIRST_WAVE].binary.mass[1]
	        / square(M[FIRST_WAVE]), parameter[SECOND_WAVE].binary.mass[0] * parameter[SECOND_WAVE].binary.mass[1]
	        / square(M[SECOND_WAVE]) };
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		fprintf(file, "#%d mass  [m1,m2,M,eta] %11.5g %11.5g %11.5g %11.5g\n", wave, parameter[wave].binary.mass[0],
		        parameter[wave].binary.mass[1], M[wave], eta[wave]);
	}
	for (int blackhole = 0; blackhole < BH; blackhole++) {
		for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
			fprintf(file, "#%d spin%d [mag,inc,azi] %11.5g %11.5g %11.5g\n", wave, blackhole,
			        parameter[wave].binary.spin.magnitude[blackhole],
			        degreeFromRadian(parameter[wave].binary.spin.inclination[blackhole]),
			        degreeFromRadian(parameter[wave].binary.spin.azimuth[blackhole]));
		}
	}
	for (int wave = FIRST_WAVE; wave < NUMBER_OF_WAVE; wave++) {
		fprintf(file, "#%d method[int, pn,amp] %11s %11d %11d\n", wave, parameter[wave].method.spin,
		        parameter[wave].method.phase, parameter[wave].method.amplitude);
	}
	fprintf(file, "#  match [typ,max,min] %11.5g %11.5g %11.5g\n", analysed->match[TYPICAL], analysed->match[BEST],
	        analysed->match[WORST]);
	fprintf(file, "#  period[ 1., 2.,rel] %11d %11d %11.5g\n", analysed->period[FIRST_WAVE],
	        analysed->period[SECOND_WAVE], analysed->relativePeriod);
	fprintf(file, "#  length[ 1., 2.,rel] %11.5g %11.5g %11.5g\n", analysed->length[FIRST_WAVE],
	        analysed->length[SECOND_WAVE], analysed->relativeLength);
}

void printSpins(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime) {
	printHeader(file, wave, analysed);
	fprintf(file, "#%10.5s ", "t");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s1x", "1s1y", "1s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2s1x", "2s1y", "2s1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1s2x", "1s2y", "1s2z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "2s2x", "2s2y", "2s2z");
	int shorter = variable->length[FIRST_WAVE] < variable->length[SECOND_WAVE] ? FIRST_WAVE : SECOND_WAVE;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g ", index * samplingTime);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S1[FIRST_WAVE][X][index],
		        variable->S1[FIRST_WAVE][Y][index], variable->S1[FIRST_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S1[SECOND_WAVE][X][index],
		        variable->S1[SECOND_WAVE][Y][index], variable->S1[SECOND_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->S2[FIRST_WAVE][X][index],
		        variable->S2[FIRST_WAVE][Y][index], variable->S2[FIRST_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", variable->S2[SECOND_WAVE][X][index],
		        variable->S2[SECOND_WAVE][Y][index], variable->S2[SECOND_WAVE][Z][index]);
	}
}

void printSystem(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime) {
	printHeader(file, wave, analysed);
	fprintf(file, "#%10.5s ", "t");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e1x", "1e1y", "1e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "2e1x", "2e1y", "2e1z");
	fprintf(file, "%11.5s %11.5s %11.5s ", "1e3x", "1e3y", "1e3z");
	fprintf(file, "%11.5s %11.5s %11.5s\n", "2e3x", "2e3y", "2e3z");
	int shorter = variable->length[FIRST_WAVE] < variable->length[SECOND_WAVE] ? FIRST_WAVE : SECOND_WAVE;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g ", index * samplingTime);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E1[FIRST_WAVE][X][index],
		        variable->E1[FIRST_WAVE][Y][index], variable->E1[FIRST_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E1[SECOND_WAVE][X][index],
		        variable->E1[SECOND_WAVE][Y][index], variable->E1[SECOND_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g ", variable->E3[FIRST_WAVE][X][index],
		        variable->E3[FIRST_WAVE][Y][index], variable->E3[FIRST_WAVE][Z][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g\n", variable->E3[SECOND_WAVE][X][index],
		        variable->E3[SECOND_WAVE][Y][index], variable->E3[SECOND_WAVE][Z][index]);
	}
}

int printOutput(FILE *file, Variable variable[2], Wave *wave, Analysed *analysed, double samplingTime) {
	printHeader(file, wave, analysed);
	fprintf(file, "#%10s %11s %11s %11s %11s %11s %11s %11s %11s %11s %11s %11s %11s\n", "t", "h1", "h2", "hp1", "hc1", "hp2",
	        "hc2", "omega1", "omega2", "phi1", "phi2", "phi1(deg)", "phi2(deg)");
	int shorter = variable->length[FIRST_WAVE] < variable->length[SECOND_WAVE] ? FIRST_WAVE : SECOND_WAVE;
	for (size_t index = 0; index < variable->length[shorter]; index++) {
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g ", index * samplingTime,
		        variable->wave->H[FIRST_WAVE][index], variable->wave->H[SECOND_WAVE][index],
		        variable->wave->h[HP1][index], variable->wave->h[HC1][index], variable->wave->h[HP2][index],
		        variable->wave->h[HC2][index]);
		fprintf(file, "% 11.5g % 11.5g % 11.5g % 11.5g % 11.5g % 11.5g\n", variable->V[FIRST_WAVE][index],
		        variable->V[SECOND_WAVE][index], variable->Phi[FIRST_WAVE][index], variable->Phi[SECOND_WAVE][index],
		        degreeFromRadian(normaliseRadians(variable->Phi[FIRST_WAVE][index])),
		        degreeFromRadian(normaliseRadians(variable->Phi[SECOND_WAVE][index])));
	}
	return (SUCCESS);
}
