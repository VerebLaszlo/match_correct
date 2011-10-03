/**
 * @file util.h
 * @author László Veréb
 * @date 2011.07.19.
 * @brief Contains useful functions.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

typedef char *string; ///< shorthand for dynamic string
typedef unsigned short ushort; ///< shorthand for unsigned short
typedef unsigned long ulong;	///< shorthand for unsigned long

typedef enum {
	ZERO = 0,
} UtilityConstants;

/**	Negates the boolean variable.
 * @param[in,out] var	: boolean variable to be negated.
 */
void neg(bool *var);

#endif // UTIL_H
