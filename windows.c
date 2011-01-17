/*
 * windows.c
 *
 *  Created on: 2011.01.12.
 *      Author: vereb
 */

#include "windows.h"

const window WINDOWS[] = { { DIRICHLET, useDirichletOn }, { BLACKMAN, useBlackmanOn } };

void useDirichletOn(double array[], size_t size, double result[]) {
	size_t i;
	for (i = 0; i < size; i++) {
		result[i] = array[i];
	}
}

void useBlackmanOn(double array[], size_t size, double result[]) {
	double length = size;
	double window, windowNormalisation = 0.;
	size_t i;
	for (i = 0; i < size; i++) {
		window = 0.42 - 0.5 * cos(2. * pi * (double) i / length) + 0.08 * cos(4. * pi * (double) i / length);
		result[i] = array[i] * window;
		windowNormalisation += SQR(window);
	}
	windowNormalisation = sqrt(windowNormalisation / length);
	for (i = 0; i < size; i++) {
		result[i] /= windowNormalisation;
	}
}
