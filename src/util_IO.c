/**
 * @file util_IO.c
 *
 * @date Aug 1, 2011
 * @author vereb
 * @brief Handles input-output specific events.
 */

#include "test.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "util_IO.h"
#ifndef NDEBUG
#include <assert.h>
#endif

extern char * program_invocation_short_name; ///< short name of the program
extern char * program_invocation_name; ///< long name of the program

/// @name File handling functions
///@{

FILE *safelyOpenFile(const char *fileName, const char *mode) {
	assert(strcmp(fileName, ""));
	assert(strcmp(mode, ""));
	FILE *stream;
	errno = 0;
	stream = fopen(fileName, mode);
	if (stream == NULL ) {
		if (strcmp(mode, "r")) {
			fprintf(stderr, "%s: Couldn't open file %s for reading; %s\n", program_invocation_short_name, fileName,
			        strerror(errno));
		} else if (strcmp(mode, "w")) {
			fprintf(stderr, "%s: Couldn't open file %s for writing; %s\n", program_invocation_short_name, fileName,
			        strerror(errno));
		} else if (strcmp(mode, "a")) {
			fprintf(stderr, "%s: Couldn't open file %s for append; %s\n", program_invocation_short_name, fileName,
			        strerror(errno));
		} else {
			fprintf(stderr, "%s: Couldn't open file %s; %s\n", program_invocation_short_name, fileName,
			        strerror(errno));
		}
		fflush(stderr);
		exit(EXIT_FAILURE);
	} else {
		return (stream);
	}
}

FILE *safelyOpenForReading(const char *fileName) {
	assert(strcmp(fileName, ""));
	return (safelyOpenFile(fileName, "r"));
}

FILE *safelyOpenForWriting(const char *fileName) {
	assert(strcmp(fileName, ""));
	return (safelyOpenFile(fileName, "w"));
}

FILE *safelyOpenForAppend(const char *fileName) {
	assert(strcmp(fileName, ""));
	return (safelyOpenFile(fileName, "a"));
}

bool makeDir(const char *name) {
	int status = mkdir(name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	if (status != EEXIST) {
		return (false);
	}
	return (true);
}

void getFileName(char *name, const char *path) {
	string temp;
	char *token;
	strcpy(temp, path);
	token = strtok(temp, "/");
	do {
		strcpy(name, token);
		token = strtok(NULL, "/");
	} while (token);
}

///@}
/// @name Output formatting functions and types
///@{

OutputFormat outputFormat[NUMBER_OF_OUTPUTFORMATS];

/**	Sets the format string for one number.
 * @param[in,out]	format	: the format
 */
static void setFormatForOneNumber(OutputFormat *format) {
	BACKUP_DEFINITION_LINE(); //
	assert(format);
	assert(format->width > 0);
	sprintf(format->empty, "%%%ds", format->width);
	if (format->leftJustified) {
		sprintf(format->oneNumber, "%%- %d.%dl%c", format->width, format->precision, format->specifier);
	} else {
		sprintf(format->oneNumber, "%% %d.%dl%c", format->width, format->precision, format->specifier);
	}; SAVE_FUNCTION_FOR_TESTING();
}

void setOutputFormat(OutputFormat *format, const ushort precision, const ushort width, const char specifier,
        const char separator, bool leftJustified) {
	BACKUP_DEFINITION_LINE(); //
	assert(format);
	assert(width < MAXIMUM_WIDTH);
	assert(precision < MAXIMUM_PRECISION);
	assert(separator);
	format->precision = precision;
	format->width =
	        (ushort) (
	                width > format->precision + SPECIAL_CHARACTER_LENGTH ?
	                        width : format->precision + SPECIAL_CHARACTER_LENGTH);
	format->widthWithSeparator = (ushort) (format->width + SEPARATOR_LENGTH);
	format->specifier = specifier;
	format->separator = separator;
	format->leftJustified = leftJustified;
	setFormatForOneNumber(format);
	SAVE_FUNCTION_FOR_TESTING();
}

static void setFormats(char formatString[], const ushort number, OutputFormat *format) {
	BACKUP_DEFINITION_LINE(); //
	assert(formatString);
	assert(number);
	assert(format);
	ushort length = (ushort) (number * format->widthWithSeparator);
	char temp[length];
	strcpy(formatString, format->oneNumber);
	if (format->separator == '%') {
		for (ushort i = 1; i < number; i++) {
			sprintf(temp, "%s %%%c %s", formatString, format->separator, format->oneNumber);
			strcpy(formatString, temp);
		}
	} else {
		for (ushort i = 1; i < number; i++) {
			sprintf(temp, "%s %c %s", formatString, format->separator, format->oneNumber);
			strcpy(formatString, temp);
		}
	}; SAVE_FUNCTION_FOR_TESTING();
}

void setFormat(char formatString[], const ushort number, OutputFormat *format) {
	BACKUP_DEFINITION_LINE(); //
	assert(formatString);
	assert(number);
	assert(format);
	setFormats(formatString, number, format);
	char temp[number * format->widthWithSeparator];
	strcpy(temp, formatString);
	if (format->separator == '%') {
		sprintf(formatString, "%s %%%c", temp, format->separator);
	} else {
		sprintf(formatString, "%s %c", temp, format->separator);
	} //
	SAVE_FUNCTION_FOR_TESTING();
}

void setFormatEnd(char formatString[], const ushort number, OutputFormat *format) {
	BACKUP_DEFINITION_LINE(); //
	assert(formatString);
	assert(number);
	assert(format);
	setFormats(formatString, number, format);
	ushort length = (ushort) (number * format->widthWithSeparator);
	char temp[length];
	strcpy(temp, formatString);
	sprintf(formatString, "%s\n", temp);
	SAVE_FUNCTION_FOR_TESTING();
}

/*
 static void printFormat(FILE *file, OutputFormat *format) {
 fprintf(file, "prefision:             %u\n", format->precision);
 fprintf(file, "width:                 %u\n", format->width);
 fprintf(file, "width with separator:  %u\n", format->widthWithSeparator);
 fprintf(file, "separator:             %c\n", format->separator);
 fprintf(file, "left jusfified:        %d\n", format->leftJustified);
 fprintf(file, "format for one number: %s\n", format->oneNumber);
 fprintf(file, "name of the format:    %s\n", format->name);
 fprintf(file, "code of the format:    %d\n", format->code);
 }
 */
///@}
#ifdef TEST

/// @name Test functions
///@{

static bool isOK_setFormatForOneNumber(void) {
	OutputFormat format;
	nameString result[2] = {"%- 10.5lg", "% 10.5lg"};
	format.leftJustified = true;
	format.precision = 5;
	format.width = 10;
	for (short i = 0; i < 2; i++, neg(&format.leftJustified)) {
		SAVE_FUNCTION_CALLER();
		setFormatForOneNumber(&format);
		if (strcmp(result[i], format.oneNumber)) {
			PRINT_ERROR();
			return (false);
		}
	}
	format.leftJustified = false;
	for (short i = 0; i < 2; i++, neg(&format.leftJustified)) {
		SAVE_FUNCTION_CALLER();
		setFormatForOneNumber(&format);
		if (!strcmp(result[i], format.oneNumber)) {
			PRINT_ERROR();
			return (false);
		}
	}
	PRINT_OK();
	return (true);
}

static bool isOK_setOutputFormat(void) {
	if (!isOK_setFormatForOneNumber()) {
		return (false);
	}
	OutputFormat format;
	char specifier = 'g';
	char separator = '%';
	ushort code = 0;
	ushort precision = 2 * SPECIAL_CHARACTER_LENGTH;
	bool leftJusfified = false;
	ushort width;
	for (ushort i = 0; i < 2; i++, neg(&leftJusfified)) {
		width = (ushort) (precision - 2 * SPECIAL_CHARACTER_LENGTH);
		for (ushort j = 0; j < 3; j++, code++) {
			SAVE_FUNCTION_CALLER();
			setOutputFormat(&format, precision, width, specifier, separator, leftJusfified);
			if (format.precision != precision) {
				PRINT_ERROR();
				return (false);
			}
			if (format.width < width) {
				PRINT_ERROR();
				return (false);
			}
			if (format.width < precision) {
				PRINT_ERROR();
				return (false);
			}
			width = (ushort) (width + 2 * SPECIAL_CHARACTER_LENGTH);
		}
	}
	/// @todo a hibás bemenet nincs leellenőrizve!!!
	PRINT_OK();
	return (true);
}

static bool isOK_setFormat(void) {
	if (!isOK_setOutputFormat()) {
		return (false);
	}
	OutputFormat format;
	char separator[] = {'%', 'X'};
	char specifier = 'g';
	ushort precision = 5;
	bool leftJusfified = false;
	ushort width = 12;
	nameString output;
	nameString result[2][2] = { {"% 12.5lg", "% 12.5lg %% % 12.5lg"}, //
		{	"% 12.5lg", "% 12.5lg X % 12.5lg"}};
	for (ushort i = 0; i < 2; i++) {
		setOutputFormat(&format, precision, width, specifier, separator[i], leftJusfified);
		SAVE_FUNCTION_CALLER();
		setFormats(output, 1, &format);
		if (strcmp(result[i][0], output)) {
			PRINT_ERROR();
			return (false);
		}
		SAVE_FUNCTION_CALLER();
		setFormats(output, 2, &format);
		if (strcmp(result[i][1], output)) {
			PRINT_ERROR();
			return (false);
		}
	}
	PRINT_OK();
	return (true);
}

bool areIOFunctionsGood(void) {
	if (isOK_setFormat()) {
		PRINT_OK_FILE();
		return (true);
	}
	PRINT_ERROR_FILE();
	return (false);
}

///@}

#endif // TEST
