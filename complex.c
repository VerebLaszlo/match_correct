#include "complex.h"

 inline void complexConjugateProduct(complex left, complex right, complex result) {
	complex temp;
	complexConjugate(right, temp);
	productComplex(left, temp, result);
}

 inline void complexConjugate(complex number, complex result) {
	result[0] = number[0];
	result[1] = -number[1];
}

 inline void productComplex(complex left, complex right, complex result) {
	result[0] = left[0] * right[0] - left[1] * right[1];
	result[1] = left[0] * right[1] + left[1] * right[0];
}

 inline void devideComplexWithDouble(complex numerator, double denominator, complex result) {
	result[0] = numerator[0] / denominator;
	result[1] = numerator[1] / denominator;
}

 inline complex* createComplexArray(size_t length) {
 	complex* array;
 	array = fftw_malloc(sizeof(complex) * length);
 	return array;
 }

 inline double* createDoubleArray(size_t length) {
 	double* array;
 	array = fftw_malloc(sizeof(double) * length);
 	return array;
 }

 inline void destroyArray(void* array) {
 	fftw_free(array);
 }
