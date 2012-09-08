/**	@file   match_fftw.h
 *	@author László Veréb
 *	@date   08.09.2012
 *	@brief  Data analysing.
 */

#ifndef MATCH_FFTW_H_
#define MATCH_FFTW_H_

void initMatch(size_t lengthFirst, size_t lengthSecond);

void prepairMatch(double *first[2], double *second[2], double *norm);

void cleanMatch(void);

void calcMatches(size_t minIndex, size_t maxIndex, double *typ, double *best, double *minimax);

#endif /* MATCH_FFTW_H_ */
