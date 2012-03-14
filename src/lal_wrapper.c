/**
 * @file lal_wrapper.c
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#include <lal/Date.h>
#include <lal/FrequencySeries.h>
#include <lal/LALSimInspiral.h>
#include <lal/LALSimNoise.h>
#include <lal/TimeSeries.h>
#include <lal/Units.h>
#include <math.h>
#include "util_math.h"
#include "lal_wrapper.h"

enum {
	HP = 0, HC,
};

/**	Gets approximant code from string.
 * @param[in] approx	: string code
 * @return
 */
static Approximant getApproximantFromString(const char * const approx) {
	Approximant result;
	if (!strcmp("SQT", approx)) {
		result = SpinQuadTaylor;
	} else if (!strcmp("STF", approx)) {
		result = SpinTaylor;
	} else {
		exit(EXIT_FAILURE);
	}
	return (result);
}

/**	Gets spin interaction code from string.
 * @param[in] spin	: string code
 * @return
 */
static LALSimInspiralInteraction getInteractionFromString(const char * const spin) {
	LALSimInspiralInteraction result = LAL_SIM_INSPIRAL_INTERACTION_NONE;
	if (strstr(spin, "ALL")) {
		result = LAL_SIM_INSPIRAL_INTERACTION_ALL_SPIN;
	} else {
		if (strstr(spin, "SO15")) {
			result |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_ORBIT_15PN;
		}
		if (strstr(spin, "SS20")) {
			result |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_2PN;
		}
		if (strstr(spin, "QM20")) {
			result |= LAL_SIM_INSPIRAL_INTERACTION_QUAD_MONO_2PN;
		}
		if (strstr(spin, "SELF")) {
			result |= LAL_SIM_INSPIRAL_INTERACTION_SPIN_SPIN_SELF_2PN;
		}
	}
	return (result);
}

/**	Creates the power spectrum density for the LIGO detector.
 * @param[out] psd_out	 :
 * @param[in]  lalparams :
 */
static void createPSD(double *psd_out, size_t length1, size_t length2, double initialFrequency,
		double samplingTime) {
	LIGOTimeGPS epoch;
	XLALGPSSetREAL8(&epoch, 0.0);
	size_t length = (size_t) length1 < length2 ? length1 : length2;
	REAL8FrequencySeries *psd = XLALCreateREAL8FrequencySeries("psd", &epoch, initialFrequency,
			length * samplingTime, &lalSecondUnit, length / 2 + 1);
	XLALSimNoisePSD(psd, initialFrequency, XLALSimNoisePSDaLIGOZeroDetHighPower);
	for (ulong current = 0; current < length; current++) {
		psd_out[current] = psd->data->data[current];
	}
	if (psd) {
		XLALDestroyREAL8FrequencySeries(psd);
	}
}

/**	Creates a signal structure and populates it from the generated waveforms.
 * @param signal
 * @param lal
 */
static void createSignalStructFromLAL(SignalStruct *signal, REAL8TimeSeries h[]) {
	signal->size = (size_t) fmax(h[0].data->length, h[1].data->length);
	if (createSignal) {
		createSignal(signal, signal->size);
	} else {
		fprintf(stderr,
				"You did not choose signal structure handling function with \"setSignalExistanceFunctions(bool)\" function.");
		exit(EXIT_FAILURE);
	}
	signal->samplingTime = h[0].deltaT;
	signal->length[H1] = h[H1].data->length;
	signal->length[H2] = h[H2].data->length;
	for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
		for (ulong current = 0; current < signal->length[i]; current++) {
			signal->componentsInTime[H1P + NUMBER_OF_SIGNALS * i][current] =
					h[2 * i].data->data[current];
			signal->componentsInTime[H1C + NUMBER_OF_SIGNALS * i][current] =
					h[2 * i + 1].data->data[current];
		}
	}
}

int generateWaveformPair(SystemParameter *parameters, SignalStruct *signal, bool calculateMatches) {
	assert(parameters);
	assert(signal);
	REAL8TimeSeries *h[NUMBER_OF_SIGNALS_COMPONENTS];
	for (ushort current = 0; current < NUMBER_OF_SYSTEMS; current++) {
		Approximant approx = getApproximantFromString(parameters->approximant[current]);
		LALSimInspiralInteraction spin = getInteractionFromString(parameters->spin[current]);
		REAL8 mass[2] = { parameters->system[current].mass.mass[0],
				parameters->system[current].mass.mass[1] };
		REAL8 s1[3] = { parameters->system[current].spin[0].component[FIXED][X],
				parameters->system[current].spin[0].component[FIXED][Y],
				parameters->system[current].spin[0].component[FIXED][Z] };
		REAL8 s2[3] = { parameters->system[current].spin[1].component[FIXED][X],
				parameters->system[current].spin[1].component[FIXED][Y],
				parameters->system[current].spin[1].component[FIXED][Z] };
		int error;
		error = XLALSimInspiralChooseWaveform(&h[2 * current], &h[2 * current + 1],
				parameters->coalescencePhase[current], parameters->samplingTime, mass[0], mass[1],
				s1[X], s1[Y], s1[Z], s2[X], s2[Y], s2[Z], parameters->initialFrequency,
				parameters->system[current].distance, parameters->system[current].inclination, 0.0,
				0.0, 1.0, 1.0, spin, parameters->amplitude[current], parameters->phase[current],
				approx);
		if (error) {
			fprintf(stderr, "Error generating %d waveform\n", current);
			return NOT_FOUND;
		}
	}
	if (signal) {
		createSignalStructFromLAL(signal, *h);
		if (calculateMatches) {
			createPSD(signal->powerSpectrumDensity, h[H1P]->data->length, h[H2P]->data->length,
					parameters->initialFrequency, parameters->samplingTime);
		}
		signal->samplingTime = parameters->samplingTime;
	}
	for (int wave = H1P; wave < NUMBER_OF_SIGNALS_COMPONENTS; wave++) {
		if (h[wave]) {
			XLALDestroyREAL8TimeSeries(h[wave]);
		}
	}
	LALCheckMemoryLeaks();
	return FOUND;
}
