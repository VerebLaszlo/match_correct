/**	@file   match_fftw.h
 *	@author László Veréb
 *	@date   08.09.2012
 *	@brief  Data analysing.
 */

#ifndef MATCH_FFTW_H_
#define MATCH_FFTW_H_

enum {
	HP1, HC1, HP2, HC2, COMPONENT, FIRST_WAVE = 0, SECOND_WAVE, NUMBER_OF_WAVE,
};

typedef struct {
	double *H[NUMBER_OF_WAVE];
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

void initMatch(Waveform *waveform);

void cleanMatch(void);

void calcMatches(size_t minIndex, size_t maxIndex, Analysed *analysed);

void countPeriods(Analysed *analysed);

#endif /* MATCH_FFTW_H_ */
