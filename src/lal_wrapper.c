/**
 * @file lal_wrapper.c
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#include <lal/LALSimInspiral.h>
#include <lal/LALMalloc.h>
#include <lal/TimeSeries.h>
#include <math.h>
#include "lal_wrapper.h"

enum {
	HP, HC,
};

static Approximant getApproximant(const char * const approximant) {
	Approximant approx = NumApproximants;
	if (strstr(approximant, "SQT")) {
		approx = SpinQuadTaylor;
	} else if (strstr(approximant, "ST")) {
		approx = SpinTaylorT4;
	}
	return (approx);
}

static LALSimInspiralInteraction getInteraction(const char * const interaction) {
	LALSimInspiralInteraction spin = LAL_SIM_INSPIRAL_INTERACTION_NONE;
	if (!strstr(interaction, "NO")) {
		if (strstr(interaction, "ALL")) {
			spin = LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN
					| LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN
					| LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN
					| LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN;
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

static void setSignalFromH(SignalStruct *signal, REAL8TimeSeries *h[2]) {
	for (ulong i = 0; i < h[HP]->data->length; i++) {
		signal->inTime[H1P][i] = h[0][HP].data->data[i];
		signal->inTime[H1C][i] = h[0][HC].data->data[i];
		signal->inTime[H2P][i] = h[1][HP].data->data[i];
		signal->inTime[H2C][i] = h[1][HC].data->data[i];
	}
}

static void createSignalFromH(SignalStruct *signal, REAL8TimeSeries *h[2]) {
	signal->size = (size_t) fmax(h[0][HP].data->length, h[1][HP].data->length);
	if (createSignal) {
		createSignal(signal, signal->size);
	} else {
		fprintf(stderr,
				"You did not choose signal structure handling function with \"setSignalExistanceFunctions(bool)\" function.");
		exit(EXIT_FAILURE);
	}
	signal->samplingTime = h[0][HP].deltaT;
	signal->length[0] = h[0][HP].data->length;
	signal->length[1] = h[1][HP].data->length;
	setSignalFromH(signal, h);
}

/** LAL parameters.
 */
typedef struct LALParameters {
//	CoherentGW waveform[NUMBER_OF_SYSTEMS]; ///<a
//	PPNParamStruc ppnParams[NUMBER_OF_SYSTEMS]; ///<a
//	RandomInspiralSignalIn randIn; ///<a
	ushort firstShorter; ///<a
	size_t length[NUMBER_OF_SYSTEMS];
	Approximant approx[NUMBER_OF_SYSTEMS];
} LALParameters;

/**	Creates the power spectrum density for the LIGO detector.
 * @param[out] psd_out	 :
 * @param[in]  lalparams :
 */
static void createPSD(double *psd_out, LALParameters *lalparams) {
	assert(lalparams);
	/*
	 REAL8Vector psd;
	 psd.length =
	 (UINT4) lalparams->ppnParams[0].length < lalparams->ppnParams[1].length ?
	 lalparams->ppnParams[0].length : lalparams->ppnParams[1].length;
	 double df = 1. / lalparams->ppnParams[0].deltaT / psd.length;
	 psd.data = (REAL8*) XLALCalloc(sizeof(REAL8), psd.length);
	 LALNoiseSpectralDensity(&lalparams->status, &psd, &LALLIGOIPsd, df);
	 for (ulong j = 0; j < psd.length; j++) {
	 psd_out[j] = psd.data[j];
	 }
	 XLALFree(psd.data);
	 */
}

int generateWaveformPair(SystemParameter *parameters, SignalStruct *signal, bool calculateMatches) {
	assert(parameters);
	assert(signal);
	REAL8TimeSeries *h[2][2] = { { NULL, NULL }, { NULL, NULL } };
	REAL8 lambda[2] = { 0.0, 0.0 };
	REAL8 qm[2] = { 1.0, 1.0 };
	REAL8 phiEnd = 0.0;
	REAL8 startingFrequency = 0.0;
	REAL8 initialFrequency = parameters->initialFrequency;
	REAL8 samplingTime = parameters->samplingTime;
	LALSimInspiralInteraction spin;
	Approximant approx;
	int amp, phase;
	REAL8 mass[2], dist, incl, chi[2][DIMENSION];
	for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
		approx = getApproximant(parameters->approximant[i]);
		spin = getInteraction(parameters->spin[i]);
		phase = parameters->phase[i];
		amp = parameters->amplitude[i];
		dist = parameters->system[i].distance;
		incl = parameters->system[i].inclination;
		mass[0] = parameters->system[i].mass.mass[0];
		mass[1] = parameters->system[i].mass.mass[1];
		for (ushort j = 0; j < 2; j++) {
			for (ushort dim = 0; dim < DIMENSION; dim++) {
				chi[j][dim] = parameters->system[i].spin[j].component[FIXED][dim];
			}
		}
		XLALSimInspiralChooseTDWaveform(&h[i][HP], &h[i][HC], phiEnd, samplingTime, mass[0],
				mass[1], chi[0][X], chi[0][Y], chi[0][Z], chi[1][X], chi[1][Y], chi[1][Z],
				initialFrequency, startingFrequency, dist, incl, lambda[0], lambda[1], qm[0], qm[1],
				spin, amp, phase, approx);
//		parameters->coalescencePhase[i] = lalparams.ppnParams[i].fStop;
		parameters->coalescenceTime[i] = h[i][HP]->data->length * samplingTime;
	}
	if (signal) {
		createSignalFromH(signal, *h);
		if (calculateMatches) {
//			createPSD(signal->powerSpectrumDensity, &lalparams);
		}
//		parameters->samplingFrequency = 1. / (lalparams.ppnParams[0].deltaT * signal->size);
		signal->samplingTime = parameters->samplingTime;
	}
	for (ushort system = 0; system < 2; system++) {
		XLALDestroyREAL8TimeSeries(h[system][HP]);
		XLALDestroyREAL8TimeSeries(h[system][HC]);
	}
	LALCheckMemoryLeaks();
	return FOUND;
}
