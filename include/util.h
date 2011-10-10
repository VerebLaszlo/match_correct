/**
 * @file util.h
 * @author László Veréb
 * @date 2011.07.19.
 * @brief Contains useful functions.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

typedef enum {
	ZERO = 0, STRING_LENGTH = 100,
} UtilityConstants;

typedef char string[STRING_LENGTH];
typedef char *stringPointer; ///< shorthand for dynamic string type
typedef const char *cstring;	//< shorthand for constant dynamic string type
typedef unsigned short ushort; ///< shorthand for unsigned short int type
typedef unsigned long ulong;

/**	Negates the boolean variable.
 * @param[in,out] var	: boolean variable to be negated.
 */
void neg(bool *var);

#endif // UTIL_H
