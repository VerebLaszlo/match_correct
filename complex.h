#ifndef COMPLEX_H_
#define COMPLEX_H_

#include <fftw3.h>

typedef fftw_complex complex;
typedef double* dp;

inline void complexConjugateProduct(complex left, complex right, complex result);
#define ComplexConjugateProduct(left, right, result) \
	({dp temp;\
	ComplexConjugate(right, temp);\
	ProductComplex(left, temp, result);})

inline void complexConjugate(complex number, complex result);
#define ComplexConjugate(number, result) \
	({dp _num = (number), _res = (result); _res[0] = _num[0]; _res[1] = -_num[1];})

inline void productComplex(complex left, complex right, complex result);
#define ProductComplex(left, right, result) \
	({dp _left = (left), _right = (right), _result = (result);\
_result[0] = _left[0] * _right[0] - _left[1] * _right[1];\
_result[1] = _left[0] * _right[1] + _left[1] * _right[0];})

inline void devideComplexWithDouble(complex numerator, double denominator, complex result);
#define DevideComplexWithDouble(numerator, denominator, result) \
	({dp num = (numerator), res = (result); double den = (denominator); res[0] = num[0] / den;	res[1] = num[1] / den;})

inline complex* createComplexArray(size_t length);

inline double* createDoubleArray(size_t length);

inline void destroyArray(void* array);

#endif /* COMPLEX_H_ */
