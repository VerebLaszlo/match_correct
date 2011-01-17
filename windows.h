#ifndef WINDOWS_H_
#define WINDOWS_H_

#include "util_math.h"

typedef enum window_enum {
	DIRICHLET, BLACKMAN,
} window_enum;

typedef struct window_table {
	window_enum id;
	void *func;
} window;

void useDirichletOn(double array[], size_t size, double result[]);

void useBlackmanOn(double array[], size_t size, double result[]);

#endif /* WINDOWS_H_ */
