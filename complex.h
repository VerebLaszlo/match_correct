#include <fftw3.h>

typedef fftw_complex complex;

complex* createComplexArray(size_t length);

double* createDoubleArray(size_t length);

void destroyArray(void* array);

void devideComplexWithDouble(complex numerator, double denominator, complex result);

#define DevideComplexWithDouble(numerator, denominator, result) \
	({complex num = (numerator), res = (result); double den = (denominator); res[0] = num[0] / den;	res[1] = num[1] / den;})

void conjugate(complex number, complex result);
#define Conjugate(number, result) \
	({complex num = (number), res = (result); res[0] = num[0]; res[1] = -num[1];

void conjugateProduct(complex left, complex right, complex result);

double conjugateProductReal(complex left, complex right);
