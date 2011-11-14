/**
 * @file signals.c
 *
 * @date 2011.08.18.
 * @author László Veréb
 */

#include "signals.h"
#include <errno.h>
#include <limits.h>

void *secureFFTWMalloc(size_t number, size_t size) {
	void *result = fftw_malloc(number * size);
	if (!result) {
		fprintf(stderr, "%s with size: %dbytes\n", strerror(errno), number * size);
		exit(EXIT_FAILURE);
	}
	return result;
}
void *secureFFTWCalloc(size_t number, size_t size) {
	void *result = fftw_malloc(number * size);
	if (!result) {
		fprintf(stderr, "%s with size: %dbytes\n", strerror(errno), number * size);
		exit(EXIT_FAILURE);
	}
	memset(result, 0, number * size);
	return result;
}

void secureFFTWFree(void *memory) {
	if (memory) {
		fftw_free(memory);
	}
}

void createSignalWithoutmatch(SignalStruct *signal, size_t size) {
	assert(signal);
	assert(size);
	memset(signal, 0, sizeof(SignalStruct));
	signal->size = size;
	for (ushort i = 0; i < NUMBER_OF_SIGNALS; i++) {
		signal->inTime[i] = secureCalloc(signal->size, sizeof(double));
	}
	for (ushort i = 0; i < NUMBER_OF_SIGNALS_COMPONENTS; i++) {
		signal->componentsInTime[i] = secureCalloc(signal->size, sizeof(double));
	}
}

void destroySignalWithoutMatch(SignalStruct *signal) {
	assert(signal);
	signal->size = 0;
	for (ushort i = 0; i < NUMBER_OF_SIGNALS; i++) {
		secureFree(signal->inTime[i]);
	}
	for (ushort i = 0; i < NUMBER_OF_SIGNALS_COMPONENTS; i++) {
		secureFree(signal->componentsInTime[i]);
	}
}

void createSignalForMatch(SignalStruct *signal, size_t size) {
	assert(signal);
	assert(size);
	memset(signal, 0, sizeof(SignalStruct));
	signal->size = size;
	size_t length = signal->size * sizeof(double);
	for (ushort i = 0; i < NUMBER_OF_SIGNALS; i++) {
		signal->inTime[i] = secureFFTWCalloc(signal->size, sizeof(double));
	}
	for (ushort i = 0; i < NUMBER_OF_SIGNALS_COMPONENTS; i++) {
		signal->componentsInFrequency[i] = secureFFTWCalloc(signal->size, sizeof(fftw_complex));
		signal->componentsInTime[i] = secureFFTWCalloc(signal->size, sizeof(double));
		signal->product[i] = secureFFTWCalloc(signal->size, sizeof(double));
		signal->plan[i] = fftw_plan_dft_r2c_1d((int) signal->size, signal->componentsInTime[i],
			signal->componentsInFrequency[i], FFTW_ESTIMATE);
	}
	signal->powerSpectrumDensity = secureFFTWCalloc(signal->size, sizeof(double));
}

void destroySignalForMatch(SignalStruct *signal) {
	for (ushort i = 0; i < NUMBER_OF_SIGNALS; i++) {
		secureFFTWFree(signal->inTime[i]);
	}
	for (ushort i = 0; i < NUMBER_OF_SIGNALS_COMPONENTS; i++) {
		if (signal->componentsInFrequency[i]) {
			secureFFTWFree(signal->componentsInFrequency[i]);
		}
		if (signal->componentsInTime[i]) {
			secureFFTWFree(signal->componentsInTime[i]);
		}
		if (signal->product[i]) {
			secureFFTWFree(signal->product[i]);
		}
		if (signal->plan[i]) {
			fftw_destroy_plan(signal->plan[i]);
		}
	}
	if (signal->powerSpectrumDensity) {
		secureFFTWFree(signal->powerSpectrumDensity);
	}
}

void (*createSignal)(SignalStruct *signal, size_t size) = NULL;
void (*destroySignal)(SignalStruct *signal) = NULL;

void setSignalExistanceFunctions(bool calculateMatch) {
	if (calculateMatch) {
		createSignal = createSignalForMatch;
		destroySignal = destroySignalForMatch;
	} else {
		createSignal = createSignalWithoutmatch;
		destroySignal = destroySignalWithoutMatch;
	}
}

void printTwoSignals(FILE *file, SignalStruct *signal) {
	assert(file);
	assert(signal);
	OutputFormat *format = &outputFormat[SIGNAL_TO_PLOT];
	ushort firstShorter = signal->length[0] < signal->length[1] ? 1 : 0;
	ushort number = 3;
	ushort length = (ushort) (number * format->widthWithSeparator);
	char formatString[length];
	setFormatEnd(formatString, number, format);
	size_t i;
	for (i = 0; i < signal->length[firstShorter]; i++) {
		fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
			signal->inTime[H2][i]);
	}
	if (firstShorter) {
		sprintf(formatString, "%s %%%c %s %%%c %s\n", format->oneNumber, format->separator,
			format->empty, format->separator, format->oneNumber);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, "",
				signal->inTime[H2][i]);
		}
	} else {
		sprintf(formatString, "%s %%%c %s %%%c %s\n", format->oneNumber, format->separator,
			format->oneNumber, format->separator, format->empty);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
				"");
		}
	}
}

void printTwoSignalsAndDifference(FILE *file, SignalStruct *signal) {
	assert(file);
	assert(signal);
	OutputFormat *format = &outputFormat[SIGNAL_TO_PLOT];
	ushort firstShorter = signal->length[0] < signal->length[1] ? 1 : 0;
	ushort number = 4;
	ushort length = (ushort) (number * format->widthWithSeparator);
	char formatString[length];
	setFormatEnd(formatString, number, format);
	size_t i;
	for (i = 0; i < signal->length[firstShorter]; i++) {
		fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
			signal->inTime[H2][i], signal->inTime[H1][i] - signal->inTime[H2][i]);
	}
	if (firstShorter) {
		sprintf(formatString, "%s %%%c %s %%%c %s %%%c %s\n", format->oneNumber, format->separator,
			format->empty, format->separator, format->oneNumber, format->separator,
			format->oneNumber);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, "",
				signal->inTime[H2][i], -signal->inTime[H2][i]);
		}
	} else {
		sprintf(formatString, "%s %%%c %s %%%c %s %%%c %s\n", format->oneNumber, format->separator,
			format->oneNumber, format->separator, format->empty, format->separator,
			format->oneNumber);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
				"", signal->inTime[H1][i]);
		}
	}
}

void printTwoSignalsWithHPHC(FILE* file, SignalStruct *signal) {
	assert(file);
	assert(signal);
	OutputFormat *format = &outputFormat[SIGNAL_TO_PLOT];
	short firstShorter = signal->length[0] < signal->length[1] ? 1 : 0;
	ushort number = 7;
	ushort length = (ushort) (number * format->widthWithSeparator);
	char formatString[length];
	setFormatEnd(formatString, number, format);
	size_t i;
	for (i = 0; i < signal->length[firstShorter]; i++) {
		fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
			signal->inTime[H2][i], signal->componentsInTime[H1P][i],
			signal->componentsInTime[H1C][i], signal->componentsInTime[H2P][i],
			signal->componentsInTime[H2C][i]);
	}
	if (firstShorter) {
		sprintf(formatString, "%s %%%c %s %%%c %s %%%c %s %%%c %s %%%c %s %%%c %s\n",
			format->oneNumber, format->separator, format->empty, format->separator,
			format->oneNumber, format->separator, format->empty, format->separator, format->empty,
			format->separator, format->oneNumber, format->separator, format->oneNumber);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, "",
				signal->inTime[H2][i], "", "", signal->componentsInTime[H2P][i],
				signal->componentsInTime[H2C][i]);
		}
	} else {
		sprintf(formatString, "%s %%%c %s %%%c %s %%%c %s %%%c %s %%%c %s %%%c %s\n",
			format->oneNumber, format->separator, format->oneNumber, format->separator,
			format->empty, format->separator, format->oneNumber, format->separator,
			format->oneNumber, format->separator, format->empty, format->separator, format->empty);
		for (; i < signal->size; i++) {
			fprintf(file, formatString, (double) i * signal->samplingTime, signal->inTime[H1][i],
				"", signal->componentsInTime[H1P][i], signal->componentsInTime[H1C][i], "", "");
		}
	}
}

void calculate_H_From_HPHC(SignalStruct *signal, double *antennaFunction) {
	for (ushort i = 0; i < 2; i++) {
		for (ulong j = 0; j < signal->length[i]; j++) {
			signal->inTime[H1 + i][j] = signal->inTime[H1P + 2 * i][j]
				* antennaFunction[H1P + 2 * i]
				+ signal->inTime[H1C + 2 * i][j] * antennaFunction[H1C + 2 * i];
		}
	}
}
