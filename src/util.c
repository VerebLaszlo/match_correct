/**
 * @file util.c
 * @author László Veréb
 * @date 2011.07.19.
 * @brief Contains useful functions.
 */

#include "util.h"

void neg(bool *var) {
	*var = !*var;
}

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *secureMalloc(size_t number, size_t size) {
	void *result = malloc(number * size);
	if (!result) {
		fprintf(stderr, "%s with size: %dbytes\n", strerror(errno), number * size);
		exit(EXIT_FAILURE);
	}
	return result;
}
void *secureCalloc(size_t number, size_t size) {
	void *result = calloc(number, size);
	if (!result) {
		fprintf(stderr, "%s with size: %dbytes\n", strerror(errno), number * size);
		exit(EXIT_FAILURE);
	}
	return result;
}

void secureFree(void *memory) {
	if (memory) {
		free(memory);
	}
}
