#include "complex.h"


complex* createComplexArray(size_t length) {
	complex* array;
	array = fftw_malloc(sizeof(complex) * length);
	return array;
}

double* createDoubleArray(size_t length) {
	double* array;
	array = fftw_malloc(sizeof(double) * length);
	return array;
}

void destroyArray(void* array) {
	fftw_free(array);
}

void devideComplexWithDouble(complex numerator, double denominator, complex result) {
	result[0] = numerator[0] / denominator;
	result[1] = numerator[1] / denominator;
}

void conjugate(complex number, complex result) {
	result[0] = number[0];
	result[1] = -number[1];
}

void conjugateProduct(complex left, complex right, complex result) {

}
