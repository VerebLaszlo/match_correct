/**	@file   match_fftw.c
 *	@author László Veréb
 *	@date   08.09.2012
 *	@brief  Data analysing.
 */
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include <stdlib.h>
#include <string.h>
#include <lal/Date.h>
#include <lal/LALSimNoise.h>
#include <lal/FrequencySeries.h>
#include <lal/Units.h>
#include "match_fftw.h"
#include "util_math.h"

#undef complex
typedef fftw_complex complex;

/**
 * Calculates inner product between to complex vectors.
 * \f[
 * 	\inProd{h_1}{h_2}=4\Re\int_{f_{min}}^{f_{max}}\frac{\tilde{h}_1(f)\tilde{h}_2^*(f)}{S_h(f)}df
 * \f]
 * @param[in] left     first vector
 * @param[in] right    second vector
 * @param[in] norm     normalising vector
 * @param[in] minIndex starting index
 * @param[in] maxIndex ending index
 * @return inner product
 */
inline static double innerProduct(complex left[], complex right[], double norm[], size_t minIndex, size_t maxIndex) {
	double scalar = 0.;
	for (size_t i = minIndex; i < maxIndex; i++) {
		//scalar += (left[i][0] * right[i][0] + left[i][1] * right[i][1]) / norm[i];
		//scalar += creal(left[i]*(~right[i])) / norm[i];
		scalar += creal(left[i] * conj(right[i])) / norm[i];
	}
	return (4.0 * scalar);
}

/**
 * Normalises the vector.
 * \f[
 * 	\tilde{e}=\frac{\tilde{h}}{\sqrt{\inProd{\tilde{h}}{\tilde{h}}}}
 * \f]
 * @param[in]  in       vector to normalise
 * @param[in]  norm     normalising vector
 * @param[in]  minIndex starting index
 * @param[in]  maxIndex ending index
 * @param[out] out      normalised vector
 */
inline static void normalise(complex *in, double *norm, size_t minIndex, size_t maxIndex, size_t length, complex *out) {
	double normalising_Constant;
	normalising_Constant = sqrt(innerProduct(in, in, norm, minIndex, maxIndex));
	for (size_t j = 0; j < length; j++) {
		out[j] /= normalising_Constant;
	}
}

/**
 * Orthogonalises the vector.
 * \f[
 * 	\tilde{e}_\bot=\tilde{e}_\times-\tilde{e}_+\frac{\inProd{\tilde{e}_+}{\tilde{e}_\times}}{\inProd{\tilde{e}_+}{\tilde{e}_+}}
 * \f]
 * @param[in]  plus     plus polarised vector
 * @param[in]  cross    cross polarised vector
 * @param[in]  norm     normalising vector
 * @param[in]  minIndex starting index
 * @param[in]  maxIndex ending index
 * @param[out] out      orthonormalised vector
 */
inline static void orthogonalise(complex *plus, complex *cross, double *norm, size_t minIndex, size_t maxIndex,
        size_t length, complex *out) {
	double pc = innerProduct(plus, cross, norm, minIndex, maxIndex);
	double constant = 1.0 / sqrt(1.0 - square(pc));
	for (size_t index = 0; index < length; index++) {
		out[index] = (cross[index] - plus[index] * pc) / constant;
	}
}

/**
 * Orthonormalises the vector.
 * @param[in]  plus     plus polarised vector
 * @param[in]  cross    cross polarised vector
 * @param[in]  norm     normalising vector
 * @param[in]  minIndex starting index
 * @param[in]  maxIndex ending index
 * @param[out] out      orthonormalised vector
 */
static void orthonormalise(complex *plus, complex *cross, double *norm, size_t minIndex, size_t maxIndex, size_t length,
        complex *out) {
	normalise(plus, norm, minIndex, maxIndex, length, plus);
	normalise(cross, norm, minIndex, maxIndex, length, cross);
	orthogonalise(plus, cross, norm, minIndex, maxIndex, length, out);
}

/**
 * Calculates cross product.
 * @param[in]  left     first vector
 * @param[in]  right    second vector
 * @param[in]  norm     normalising vector
 * @param[in]  minIndex starting index
 * @param[in]  maxIndex ending index
 * @param[out] out      cross product
 */
inline static void crossProduct(complex left[], complex right[], double norm[], size_t minIndex, size_t maxIndex,
        complex out[]) {
	for (size_t i = minIndex; i < maxIndex; i++) {
		out[i] = 4.0 * left[i] * conj(right[i]) / norm[i];
	}
}

enum {
	PP, PC, CP, CC, PRODUCT,
};

static void matches(double *product[PRODUCT], size_t size, double *typ, double *best, double *minimax) {
	double A, B, C;
	double match_typ, max_Typ = 0.0;
	double match_best, max_Best = 0.0;
	double match_minimax, max_Minimax = 0.0;
	for (size_t index = 0; index < size; index++) {
		A = square(product[PP][index]) + square(product[PC][index]);
		B = square(product[CP][index]) + square(product[CC][index]);
		C = product[PP][index] * product[CP][index] + product[PC][index] * product[CC][index];
		match_typ = sqrt(A);
		max_Typ = max_Typ > match_typ ? max_Typ : match_typ;
		match_best = sqrt((A + B) / 2. + sqrt(square(A - B) / 4. + square(C)));
		max_Best = max_Best > match_best ? max_Best : match_best;
		match_minimax = sqrt((A + B) / 2. - sqrt(square(A - B) / 4. + square(C)));
		max_Minimax = max_Minimax > match_minimax ? max_Minimax : match_minimax;
	}
	*typ = max_Typ / 2.;
	*best = max_Best / 2.;
	*minimax = max_Minimax / 2.;
}

inline static size_t max(size_t first, size_t second) {
	return (first > second ? first : second);
}

Waveform *createWaveform(size_t firstLength, size_t secondLength) {
	Waveform *waveform = calloc(1, sizeof(Waveform));
	waveform->length[HP] = firstLength;
	waveform->length[HC] = secondLength;
	waveform->size = max(waveform->length[HP], waveform->length[HC]);
	for (int wave = HP; wave < WAVE; wave++) {
		waveform->H[wave] = calloc(waveform->size, sizeof(double));
	}
	for (int wave = HP1; wave < COMPONENT; wave++) {
		waveform->h[wave] = fftw_alloc_real(waveform->size);
	}
	return (waveform);
}

void destroyWaveform(Waveform **waveform) {
	for (int wave = HP; wave < WAVE; wave++) {
		free((*waveform)->H[wave]);
	}
	for (int wave = HP1; wave < COMPONENT; wave++) {
		fftw_free((*waveform)->h[wave]);
	}
	free(*waveform);
}

typedef struct {
	Waveform *wave;
	fftw_plan plan[COMPONENT];
	complex *inFrequency[COMPONENT];
	complex *product;
	fftw_plan iplan[COMPONENT];
	double *correlated[COMPONENT];
	double *norm;
	size_t length[2];
	size_t size;
} Data;

Data data;

void indexFromFrequency(double min, double max, double step, size_t *minIndex, size_t *maxIndex) {
	*minIndex = *maxIndex = 0;
	double fr = 0.;
	while (fr < min) {
		fr += step;
		*maxIndex = ++(*minIndex);
	}
	while (fr < max) {
		fr += step;
		(*maxIndex)++;
	}
}

void generatePSD(double initialFrequency, double samplingFrequency) {
	LIGOTimeGPS epoch;
	XLALGPSSetREAL8(&epoch, 1.0);
	REAL8FrequencySeries *psd = XLALCreateREAL8FrequencySeries("aLIGO", &epoch, initialFrequency,
	        samplingFrequency / data.size, &lalSecondUnit, data.size);
	XLALSimNoisePSD(psd, initialFrequency, XLALSimNoisePSDaLIGOHighFrequency);
	memcpy(data.norm, psd->data->data, data.size * sizeof(double));
	XLALDestroyREAL8FrequencySeries(psd);
}

void initMatch(size_t lengthFirst, size_t lengthSecond) {
	data.length[0] = lengthFirst;
	data.length[1] = lengthSecond;
	data.size = max(data.length[0], data.length[1]);
	data.product = fftw_alloc_complex(data.size);
	for (int wave = HP1; wave < COMPONENT; wave++) {
		data.inFrequency[wave] = fftw_alloc_complex(data.size);
		data.plan[wave] = fftw_plan_dft_r2c_1d((int) data.size, data.wave->h[wave], data.inFrequency[wave],
		        FFTW_ESTIMATE);
		data.correlated[wave] = fftw_alloc_real(data.size);
		data.iplan[wave] = fftw_plan_dft_c2r_1d((int) data.size, data.product, data.correlated[wave], FFTW_ESTIMATE);
		memset(data.inFrequency[wave], 0, data.size * sizeof(complex));
		memset(data.correlated[wave], 0, data.size * sizeof(double));
	}
	memset(data.product, 0, data.size * sizeof(double));
	data.norm = fftw_alloc_real(data.size);
}

void prepairMatch(Waveform *waveform, double *norm) {
	//size_t size = data.size * sizeof(double);
	//memset(data.norm, 0, size);
	//memcpy(data.norm, norm, size);
	data.wave = waveform;
}

void cleanMatch(void) {
	for (int wave = HP1; wave < COMPONENT; wave++) {
		fftw_free(data.inFrequency[wave]);
		fftw_destroy_plan(data.plan[wave]);
		fftw_destroy_plan(data.iplan[wave]);
	}
	fftw_free(data.norm);
	fftw_free(data.product);
}

void calcMatches(size_t minIndex, size_t maxIndex, Analysed *analysed) {
	for (int wave = HP1; wave < COMPONENT; wave++) {
		fftw_execute(data.plan[wave]);
	}
	for (int wave = HP1; wave < HP2; wave++) {
		orthonormalise(data.inFrequency[2 * wave], data.inFrequency[2 * wave + 1], data.norm, minIndex, maxIndex,
		        data.size, data.inFrequency[2 * wave + 1]);
	}
	for (int wave = HP1; wave < COMPONENT; wave++) {
		memset(data.product, 0, data.size * sizeof(complex));
		crossProduct(data.inFrequency[wave / 2], data.inFrequency[wave % 2 + 2], data.norm, minIndex, maxIndex,
		        data.product);
		fftw_execute(data.iplan[wave]);
	}
	matches(data.correlated, data.size, &analysed->match[TYPICAL], &analysed->match[BEST], &analysed->match[WORST]);
}

void countPeriods(Analysed *analysed) {
	for (ushort wave = 0; wave < 2; wave++) {
		analysed->period[wave] = 0;
		for (size_t index = 1; index < data.length[wave]; index++) {
			double product = data.wave->H[wave][index - 1] * data.wave->H[wave][index];
			if (product < 0.0) {
				analysed->period[wave]++;
			} else if (product == 0.0) {
				analysed->period[wave]++;
				index++;
			}
		}
		analysed->period[wave]--;
		analysed->period[wave] /= 2;
	}
}
