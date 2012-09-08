/**	@file   match_fftw.c
 *	@author László Veréb
 *	@date   08.09.2012
 *	@brief
 */
#include <math.h>
#include <complex.h>
#include <fftw3.h>
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
inline static void normalise(complex *in, double *norm, size_t minIndex, size_t maxIndex, complex *out) {
	double normalising_Constant;
	normalising_Constant = sqrt(innerProduct(in, in, norm, minIndex, maxIndex));
	for (size_t j = minIndex; j < maxIndex; j++) {
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
        complex *out) {
	double pp = innerProduct(plus, plus, norm, minIndex, maxIndex);
	double pc = innerProduct(plus, cross, norm, minIndex, maxIndex);
	for (size_t index = minIndex; index < maxIndex; index++) {
		out[index] = cross[index] - plus[index] * pc / pp;
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
static void orthonormalise(complex *plus, complex *cross, double *norm, size_t minIndex, size_t maxIndex,
        complex *out) {
	normalise(plus, norm, minIndex, maxIndex, plus);
	normalise(cross, norm, minIndex, maxIndex, cross);
	orthogonalise(plus, cross, norm, minIndex, maxIndex, out);
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
