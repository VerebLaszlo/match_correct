/*
 * util_math.h
 *
 *  Created on: 2011.01.12.
 *      Author: vereb
 */

#ifndef UTIL_MATH_H_
#define UTIL_MATH_H_

#include <string.h>
#include <math.h>

#define pi M_PI

#define SQR(A)\
	({typeof(A) _A = A; _A*_A;})

#endif /* UTIL_MATH_H_ */
