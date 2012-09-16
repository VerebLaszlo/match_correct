/**	@file   match_fftw.h
 *	@author László Veréb
 *	@date   08.09.2012
 *	@brief  Data analysing.
 */

#ifndef MATCH_FFTW_H_
#define MATCH_FFTW_H_

enum {
	HP, HC, WAVE, HP1 = 0, HC1, HP2, HC2, COMPONENT,
};

typedef struct {
	double *H[WAVE];
	double *h[COMPONENT];
	size_t length[2];
	size_t size;
} Waveform;

Waveform *createWaveform(size_t firstLength, size_t secondLength);

void destroyWaveform(Waveform **waveform);

enum {
	WORST, TYPICAL, BEST, MATCH,
};

typedef struct {
	double match[MATCH];
	size_t period[2];
} Analysed;

void indexFromFrequency(double min, double max, double step, size_t *minIndex, size_t *maxIndex);

void generatePSD(double initialFrequency, double samplingFrequency);

void initMatch(size_t lengthFirst, size_t lengthSecond);

void prepairMatch(Waveform *waveform, double *norm);

void cleanMatch(void);

void calcMatches(size_t minIndex, size_t maxIndex, Analysed *analysed);

void countPeriods(Analysed *analysed);

#endif /* MATCH_FFTW_H_ */
