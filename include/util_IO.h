/**
 * @file util_IO.h
 *
 * @date Aug 1, 2011
 * @author vereb
 * @brief Handles input-output specific events.
 */

#ifndef UTIL_IO_H_
#define UTIL_IO_H_

#include <stdio.h>
#include "util.h"

/// @name File handling functions
///@{

/**	Opens the file with the given access mode. On error it terminates the program and prints an
 * error message.
 * @param[in]	fileName	: the file's path name relative to the program
 * @param[in]	mode		: the mode with which the file is opened
 * @return	pointer to the opened file
 */
FILE *safelyOpenFile(const char *fileName, const char *mode);

/**	Opens the file just for reading. On error it terminates the program and prints an error
 * message.
 * @param[in]	fileName	: the file's path name relative to the program
 * @return	pointer to the opened file
 */
FILE *safelyOpenForReading(const char *fileName);

/**	Opens the file just for writing. On error it terminates the program and prints an error
 * message.
 * @param[in]	fileName	: the file's path name relative to the program
 * @return	pointer to the opened file
 */
FILE *safelyOpenForWriting(const char *fileName);

/**	Opens the file just for append. On error it terminates the program and prints an error
 * message.
 * @param[in]	fileName	: the file's path name relative to the program
 * @return	pointer to the opened file
 */
FILE *safelyOpenForAppend(const char *fileName);

bool makeDir(const char *name);

void getFileName(char *name, const char *path);

///@}
/// @name Output formatting functions and types
///@{

/** Constants for output
 */
typedef enum OUTPUT_CONSTANTS {
	SPECIAL_CHARACTER_LENGTH = 6, ///< number of the special characters in the format string: sign, exponents
	MAXIMUM_WIDTH = 99, ///< maximum width of the number
	MAXIMUM_PRECISION = MAXIMUM_WIDTH - SPECIAL_CHARACTER_LENGTH, ///< maximum precision ot the number
	SEPARATOR_LENGTH = 3, ///< length of the separator string: separator character and two spaces on the sides
	FORMAT_LENGTH = 11,	///< length fo the format string
	NAMES_LENGTH = 100,
///< maximum length of the names
} OUTPUT_CONSTANTS;	///<

typedef char nameString[NAMES_LENGTH]; ///< shorthand for string containing names

typedef enum {
	SIGNAL_TO_PLOT, SIGNAL_DATA, DATA, OTHER, NUMBER_OF_OUTPUTFORMATS,
} OutputFormats;

/**	Contains values to format an output.
 */
typedef struct tagOutputFormat {
	ushort precision; ///< how many digits are after the decimal point
	ushort width; ///< width of the format including the sign and exponent
	ushort widthWithSeparator; ///< width of the format including the sign, exponent and separator character
	char specifier;	///< specifies withc convention to use: 'e' - scientific, 'f' - floating point, 'g' - the shorter
	char separator; ///< column separator for gnuplot
	bool leftJustified; ///< it's true if the format is left justified, otherwise false
	char oneNumber[FORMAT_LENGTH]; ///< format string form one number
	char empty[FORMAT_LENGTH];
} OutputFormat;

extern OutputFormat outputFormat[NUMBER_OF_OUTPUTFORMATS];

/**	Sets the format variables.
 * @param[out]	format			: format specific variables
 * @param[in]	precision		: user supplied precision of the format
 * @param[in]	width			: user supplied width of the format, its has a minimal value
 * @param[in]	separator		: user defined separator character
 * @param[in]	leftJustified	: left (true) or right (false) justified text
 */
void setOutputFormat(OutputFormat *format, const ushort precision, const ushort width,
	const char specifier, const char separator, bool leftJustified);

/**	Set the format for given number of floating pint data to display.
 * @param[out] formatString	: the generated format string
 * @param[in] number		: number of the data
 * @param[in] format		: contains the format variables
 */
void setFormat(char formatString[], const ushort number, OutputFormat *format);

void setFormatEnd(char formatString[], const ushort number, OutputFormat *format);

///@}
#ifdef TEST
/// @name Test functions
///@{

/**	Tests if the input/output functions are correctly written.
 * @return true or false
 */bool areIOFunctionsGood(void);

///@}

#endif // TEST
///@}

#endif /* UTIL_IO_H_ */
