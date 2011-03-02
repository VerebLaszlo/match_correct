/**
 * @file match_qmss.c
 *
 * @date Feb 25, 2011
 * @author vereb
 */

#include "match_qmss.h"

short is_First;///<a
long db;

void run_Algorithm(Program_Parameters *program_Parameters, System_Parameters *parameters,
		binary_System *limits) {
	assert(program_Parameters);
	assert(parameters);
	assert(limits);
	assert(program_Parameters->number_Of_Runs > 0);
	srand(86);
	short is_Good;
	char temp[FILE_NAME_LENGTH];
	sprintf(temp, "%s", program_Parameters->folder);
	for (long i = 0; i < program_Parameters->number_Of_Runs;) {
		generate_Parameters(parameters, limits);
		is_Good = incrementing_Spins(program_Parameters, parameters);
		if (is_Good) {
			i++;
			db = i;
		}
		if ((i + 1) % 10 == 0) {
			printf("%ld%%\n", 100 * i / program_Parameters->number_Of_Runs);
		}
		sprintf(program_Parameters->folder, "%s", temp);
	}
}

void generate_Parameters(System_Parameters *parameters, binary_System *limits) {
	assert(parameters);
	assert(limits);
	gen_Parameters(&parameters->system[0], &limits[0], &limits[1], ETAM, KAPPA_PSI);
	memcpy(&parameters->system[1], &parameters->system[0], sizeof(binary_System));
}

short incrementing_Spins(Program_Parameters *prog, System_Parameters* parameters) {
	assert(prog);
	assert(parameters);
	char temp[FILE_NAME_LENGTH];
	sprintf(temp, "%s", prog->folder);
	is_First = 1;
	short is_Good;
	parameters->critical_Match = 0.0;
	for (; parameters->system[MOD_SPIN_INDEX].bh[0].chi_Amp < parameters->max_Spin; parameters->system[MOD_SPIN_INDEX].bh[0].chi_Amp
			+= parameters->spin_Step) {
		for (; parameters->system[MOD_SPIN_INDEX].bh[1].chi_Amp < parameters->max_Spin; parameters->system[MOD_SPIN_INDEX].bh[1].chi_Amp
				+= parameters->spin_Step) {
			is_Good = calc_Matches_For_ParameterPair(prog, parameters);
			if (is_First) {
				if (!is_Good) {
					return is_Good;
				}
				parameters->critical_Match = parameters->match_Minimax;
				is_First = 0;
			}
			sprintf(prog->folder, "%s", temp);
		}
	}
	return is_Good;
}

inline void increment_Spin_Of_Binary_System(binary_System *system, double step) {
	assert(system);
	assert(step>0.);
	system->bh[0].chi_Amp += step;
	system->bh[1].chi_Amp += step;
}

inline void increment_Spins(System_Parameters* parameters) {
	assert(parameters);
	increment_Spin_Of_Binary_System(&parameters->system[0], parameters->spin_Step);
	increment_Spin_Of_Binary_System(&parameters->system[1], parameters->spin_Step);
}

short calc_Matches_For_ParameterPair(Program_Parameters *prog, System_Parameters *parameters) {
	assert(prog);
	assert(parameters);
	static LALParameters lalparams;
	//	double f0, f1;
	signalStruct sig;
	initLALParameters(&lalparams, parameters);
	for (short i = 0; i < 2; i++) {
		memset(&lalparams.status, 0, sizeof(LALStatus));
		LALGenerateInspiral(&lalparams.status, &lalparams.waveform[i], &lalparams.injParams[i],
				&lalparams.ppnParams);
		if (lalparams.status.statusCode) {
			fprintf(stderr, "%d: LALSQTPNWaveformTest: error generating waveform\n", i);
			XLALSQTPNDestroyCoherentGW(&lalparams.waveform[0]);
			return 1;
		}
		parameters->system[i].coaPhase
				= lalparams.waveform[i].phi->data->data[lalparams.waveform[i].phi->data->length - 1];
		parameters->system[i].coaTime = (lalparams.waveform[i].f->data->length - 1)
				* parameters->time_Sampling;
	}
	parameters->shorter = lalparams.shorter = lalparams.waveform[0].f->data->length
			< lalparams.waveform[1].f->data->length ? 0 : 1;
	parameters->min_Length = lalparams.min_Length
			= lalparams.waveform[lalparams.shorter].f->data->length;
	parameters->max_Length = lalparams.max_Length
			= lalparams.waveform[!lalparams.shorter].f->data->length;
	parameters->freq_Max = (lalparams.injParams[0].f_final + lalparams.injParams[1].f_final) / 2.;
	parameters->freq_Step = 1. / (lalparams.ppnParams.deltaT * lalparams.max_Length);
	create_Signal_Struct(&sig, lalparams.waveform[!lalparams.shorter].f->data->length);
	createPSD(&lalparams, &sig);
	for (short i = 0; i < 2; i++) {
		for (long j = 0; j < lalparams.waveform[i].f->data->length; j++) {
			sig.signal[2 * i][j] = lalparams.waveform[i].h->data->data[2 * j];
			sig.signal[2 * i + 1][j] = lalparams.waveform[i].h->data->data[2 * j + 1];
		}
	}
	double fr = 0.;
	long minfr = 0, maxfr = 0;
	while (fr < parameters->freq_Min) {
		fr += parameters->freq_Step;
		maxfr = ++minfr;
	}
	while (fr < parameters->freq_Max) {
		fr += parameters->freq_Step;
		maxfr++;
	}
	parameters->match_Typ = parameters->match_Best = parameters->match_Minimax = 0.0;
	calc_Matches(&sig, minfr, maxfr, &parameters->match_Typ, &parameters->match_Best,
			&parameters->match_Minimax);
	if (parameters->match_Minimax > parameters->min_Match) {
		write_Waves_To_Files(prog, parameters, &sig);
	}
	XLALSQTPNDestroyCoherentGW(&lalparams.waveform[0]);
	XLALSQTPNDestroyCoherentGW(&lalparams.waveform[1]);
	destroy_Signal_Struct(&sig);
	XLALFree(lalparams.randIn.psd.data);
	if (parameters->match_Minimax > parameters->min_Match) {
		return 1;
	}
	return 0;
}

void initLALParameters(LALParameters *lalparams, System_Parameters *parameters) {
	assert(lalparams);
	assert(parameters);
	memset(&lalparams->waveform, 0, 2 * sizeof(CoherentGW));
	memset(&lalparams->injParams, 0, 2 * sizeof(SimInspiralTable));
	memset(&lalparams->ppnParams, 0, sizeof(PPNParamStruc));
	memset(&lalparams->waveform, 0, 2 * sizeof(CoherentGW));
	lalparams->ppnParams.deltaT = 1. / parameters->freq_Sampling;
	parameters->freq_Min = 40.;
	for (short i = 0; i < 2; i++) {
		lalparams->injParams[i].mass1 = parameters->system[i].bh[0].m;
		lalparams->injParams[i].mass2 = parameters->system[i].bh[1].m;
		lalparams->injParams[i].spin1x = parameters->system[i].bh[0].chi[0];
		lalparams->injParams[i].spin1y = parameters->system[i].bh[0].chi[1];
		lalparams->injParams[i].spin1z = parameters->system[i].bh[0].chi[2];
		lalparams->injParams[i].spin2x = parameters->system[i].bh[1].chi[0];
		lalparams->injParams[i].spin2y = parameters->system[i].bh[1].chi[1];
		lalparams->injParams[i].spin2z = parameters->system[i].bh[1].chi[2];
		lalparams->injParams[i].inclination = parameters->system[i].incl;
		lalparams->injParams[i].f_lower = parameters->freq_Min;
		lalparams->injParams[i].distance = parameters->system[i].dist;
		lalparams->injParams[i].coa_phase = parameters->system[i].coaPhase = 0.;
		lalparams->injParams[i].f_lower = parameters->freq_Initial;
		lalparams->ppnParams.deltaT = 1. / parameters->freq_Sampling;
		lalparams->injParams[i].amp_order = parameters->amp_Code;
		snprintf(lalparams->injParams[i].waveform, LIGOMETA_WAVEFORM_MAX * sizeof(CHAR), "%s%s%s",
				parameters->approx[i], parameters->phase[i], parameters->spin[i]);
	}
}

void createPSD(LALParameters *lalparams, signalStruct *sig) {
	assert(lalparams);
	assert(sig);
	lalparams->randIn.psd.length = lalparams->max_Length;
	double df = 1. / lalparams->ppnParams.deltaT / lalparams->randIn.psd.length;
	lalparams->randIn.psd.data = (REAL8*) LALMalloc(sizeof(REAL8) * lalparams->randIn.psd.length);
	LALNoiseSpectralDensity(&lalparams->status, &lalparams->randIn.psd, &LALLIGOIPsd, df);
	for (long j = 0; j < lalparams->randIn.psd.length; j++) {
		sig->psd[j] = lalparams->randIn.psd.data[j];
	}
}
