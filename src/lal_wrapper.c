/**
 * @file lal_wrapper.c
 *
 * @date Apr 9, 2011
 * @author vereb
 */

#include <lal/LALSQTPNWaveformInterface.h>
#include <lal/LALNoiseModelsInspiral.h>
#include <lal/SimulateCoherentGW.h>
#include <lal/GenerateInspiral.h>
#include <lal/LALInspiralBank.h>
#include <lal/LALDatatypes.h>
#include <lal/LALInspiral.h>
#include <lal/LALStdlib.h>
#include <lal/RealFFT.h>
#include "lal_wrapper.h"

int switchMode = LALSQTPN_PRECESSING;

/** LAL parameters.
 */
typedef struct LALParameters {
	LALStatus status; ///<a
	CoherentGW waveform[NUMBER_OF_SYSTEMS]; ///<a
	SimInspiralTable injParams[NUMBER_OF_SYSTEMS]; ///<a
	PPNParamStruc ppnParams[NUMBER_OF_SYSTEMS]; ///<a
	RandomInspiralSignalIn randIn; ///<a
	ushort firstShorter; ///<a
	size_t length[NUMBER_OF_SYSTEMS];
	Approximant approx[NUMBER_OF_SYSTEMS];
} LALParameters;

/**	Converts the amplitude character code to number code.
 * @param[in] amplitudeOrder :
 * @return
 */
static INT4 convertAmplitudeOrderFromString(const char *amplitudeOrder) {
	INT4 amplitudeCode;
	if (!strstr(amplitudeOrder, "00PN")) {
		amplitudeCode = 0;
	} else if (!strstr(amplitudeOrder, "05PN")) {
		amplitudeCode = 1;
	} else if (!strstr(amplitudeOrder, "10PN")) {
		amplitudeCode = 2;
	} else {
		amplitudeCode = -1;
	}
	return amplitudeCode;
}

/**	Initialize the parameters for calling the lal functions.
 * @param[out] lalparams  :
 * @param[in]  parameters :
 */
static void initLALParameters(LALParameters *lalparams, SystemParameter *parameters) {
	assert(lalparams);
	assert(parameters);
	memset(lalparams, 0, sizeof(LALParameters));
	for (short i = 0; i < 2; i++) {
		lalparams->injParams[i].mass1 = (REAL4) parameters->system[i].mass.mass[0];
		lalparams->injParams[i].mass2 = (REAL4) parameters->system[i].mass.mass[0];
		lalparams->injParams[i].spin1x = (REAL4) parameters->system[i].spin[0].component[FIXED][X];
		lalparams->injParams[i].spin1y = (REAL4) parameters->system[i].spin[0].component[FIXED][Y];
		lalparams->injParams[i].spin1z = (REAL4) parameters->system[i].spin[0].component[FIXED][Z];
		lalparams->injParams[i].spin2x = (REAL4) parameters->system[i].spin[1].component[FIXED][X];
		lalparams->injParams[i].spin2y = (REAL4) parameters->system[i].spin[1].component[FIXED][Y];
		lalparams->injParams[i].spin2z = (REAL4) parameters->system[i].spin[1].component[FIXED][Z];
		lalparams->injParams[i].inclination = (REAL4) parameters->system[i].inclination;
		lalparams->injParams[i].f_lower = (REAL4) parameters->initialFrequency;
		lalparams->injParams[i].distance = (REAL4) parameters->system[i].distance;
		lalparams->ppnParams[i].deltaT = 1. / parameters->samplingFrequency;
		lalparams->injParams[i].amp_order = convertAmplitudeOrderFromString(
			parameters->amplitude[i]);
		snprintf(lalparams->injParams[i].waveform, LIGOMETA_WAVEFORM_MAX * sizeof(CHAR), "%s%s%s",
			parameters->approximant[i], parameters->phase[i], parameters->spin[i]);
		if (strstr(parameters->approximant[i], "SpinQuadTaylor")) {
			lalparams->approx[i] = SpinQuadTaylor;
		} else if (strstr(parameters->approximant[i], "SpinTaylorFrameless")) {
			lalparams->approx[i] = SpinTaylorFrameless;
		} else if (strstr(parameters->approximant[i], "SpinTaylor")) {
			lalparams->approx[i] = SpinTaylor;
		}
	}
}

/**	Creates the power spectrum density for the LIGO detector.
 * @param[out] psd_out	 :
 * @param[in]  lalparams :
 */
static void createPSD(double *psd_out, LALParameters *lalparams) {
	assert(lalparams);
	REAL8Vector psd;
	psd.length = (UINT4) lalparams->length[!lalparams->firstShorter];
	double df = 1. / lalparams->ppnParams[0].deltaT / psd.length;
	psd.data = (REAL8*) XLALCalloc(sizeof(REAL8), psd.length);
	LALNoiseSpectralDensity(&lalparams->status, &psd, &LALLIGOIPsd, df);
	for (ulong j = 0; j < psd.length; j++) {
		psd_out[j] = psd.data[j];
	}
	XLALFree(psd.data);
}

/**	Sets the signal components from the generated waveform.
 * @param[out] signal	:
 * @param[in]  length	:
 * @param[in]  waveform :
 */
static void setSignalFromA1A2(double *signal[], size_t length, CoherentGW *waveform) {
	REAL8 a1, a2, phi, shift;
	for (ulong j = 0; j < length; j++) {
		a1 = waveform->a->data->data[2 * j];
		a2 = waveform->a->data->data[2 * j + 1];
		phi = waveform->phi->data->data[j];
		shift = 0.0;
		signal[0][j] = a1 * cos(shift) * cos(phi) - a2 * sin(shift) * sin(phi);
		signal[1][j] = a1 * sin(shift) * cos(phi) + a2 * cos(shift) * sin(phi);
	}
}

/**	Sets the signal components from the generated waveform.
 * @param[out] signal	:
 * @param[in]  length	:
 * @param[in]  waveform :
 */
static void setSignalsFromH(double *signal[], size_t length, CoherentGW *waveform) {
	for (ulong j = 0; j < length; j++) {
		signal[0][j] = waveform->h->data->data[2 * j];
		signal[1][j] = waveform->h->data->data[2 * j + 1];
	}
}

/**	Creates a signal structure and populates it from the generated waveforms.
 * @param signal
 * @param lal
 */
static void createSignalStructFromLAL(SignalStruct *signal, LALParameters *lal) {
	signal->size = (size_t) fmax(lal->ppnParams[0].length, lal->ppnParams[1].length);
	createSignal(signal, signal->size);
	signal->length[0] = lal->ppnParams[0].length;
	signal->length[1] = lal->ppnParams[1].length;
	for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
		switch (lal->approx[i]) {
		case SpinQuadTaylor:
			setSignalsFromH(&signal->componentsInTime[H1P + NUMBER_OF_SIGNALS * i],
				signal->length[i], &lal->waveform[i]);
			break;
		case SpinTaylorFrameless:
			setSignalsFromH(&signal->componentsInTime[H1P + NUMBER_OF_SIGNALS * i],
				signal->length[i], &lal->waveform[i]);
			break;
		case SpinTaylor:
			setSignalFromA1A2(&signal->componentsInTime[H1P + NUMBER_OF_SIGNALS * i],
				signal->length[i], &lal->waveform[i]);
			break;
		case TaylorT1:
		case TaylorT2:
		case TaylorT3:
		case TaylorF1:
		case TaylorF2:
		case PadeT1:
		case PadeF1:
		case EOB:
		case BCV:
		case BCVSpin:
		case SpinTaylorT3:
		case PhenSpinTaylorRD:
		case PhenSpinTaylorRDF:
		case FindChirpSP:
		case FindChirpPTF:
		case GeneratePPN:
		case BCVC:
		case FrameFile:
		case AmpCorPPN:
		case NumRel:
		case Eccentricity:
		case EOBNR:
		case EOBNRv2:
		case EOBNRv2HM:
		case IMRPhenomA:
		case IMRPhenomB:
		case IMRPhenomFA:
		case IMRPhenomFB:
		case TaylorEt:
		case TaylorT4:
		case TaylorN:
		case NumApproximants:
		default:
			break;
		}
	}
}

int generateWaveformPair(SystemParameter *parameters, SignalStruct *signal) {
	assert(parameters);
	assert(signal);
	static LALParameters lalparams;
	initLALParameters(&lalparams, parameters);
	for (ushort i = 0; i < NUMBER_OF_SYSTEMS; i++) {
		memset(&lalparams.status, 0, sizeof(LALStatus));
		LALGenerateInspiral(&lalparams.status, &lalparams.waveform[i], &lalparams.injParams[i],
			&lalparams.ppnParams[i]);
		if (lalparams.status.statusCode) {
			fprintf(stderr, "Error generating %d waveform\n", i);
			XLALSQTPNDestroyCoherentGW(&lalparams.waveform[0]);
			return NOT_FOUND;
		}
		parameters->coalescencePhase[i] = lalparams.ppnParams[i].fStop;
		parameters->coalescenceTime[i] = lalparams.ppnParams[i].tc;
	}
	if (signal) {
		createSignalStructFromLAL(signal, &lalparams);
		createPSD(signal->powerSpectrumDensity, &lalparams);
		parameters->samplingFrequency = 1. / (lalparams.ppnParams[0].deltaT * signal->size);
		signal->samplingTime = parameters->samplingTime;
	}
	XLALSQTPNDestroyCoherentGW(&lalparams.waveform[0]);
	XLALSQTPNDestroyCoherentGW(&lalparams.waveform[1]);
	LALCheckMemoryLeaks();
	return FOUND;
}
