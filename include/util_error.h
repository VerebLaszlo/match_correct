/**	@file	util_error.h
 *	@author vereb
 *	@date	12 Jul 2012
 *	@brief	
 */

#ifndef UTIL_ERROR_H_
#define UTIL_ERROR_H_

#include "stdio.h"

#define HERE()	\
	fprintf(stderr, "%s %d in %s\n", __FILE__, __LINE__, __func__)

#define HERED(D)	\
	fprintf(stderr, "%s %d in %s:\t\t\t%d\n", __FILE__, __LINE__, __func__, D)

#define HEREG(G)	\
	fprintf(stderr, "%s %d in %s:\t\t\t%g\n", __FILE__, __LINE__, __func__, G)

#define HERES(S)	\
	fprintf(stderr, "%s %d in %s:\t\t\t%s\n", __FILE__, __LINE__, __func__, S)

#define HEREM(F, A...) ({\
	char debug[] = "%s %d in %s:\t\t\t%s";	\
	char format[100];	\
	sprintf(format, debug, F);	\
	fprintf(stderr, format, __FILE__, __LINE__, __func__, A);})

#endif /* UTIL_ERROR_H_ */
