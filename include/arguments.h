/**
 * @file arguments.h
 * @author vereb
 * @date Aug 10, 2011
 * @brief 
 */

#ifndef ARGUMENTS_H_
#define ARGUMENTS_H_

#include "util.h"
#include <stdbool.h>

#define FILENAME_MAX 100
#define SHORT_OPTION_LENGTH 2
#define LONG_OPTION_LENGTH 50

typedef enum _OptionCode {
	NONE, HELP, SYSTEM_FILE, PARAMETER_FILE, NO_MORE_PARAMETER_CODE,
} OptionCode;

typedef struct _OptionPairs {
	OptionCode code;
	char shortOption[SHORT_OPTION_LENGTH];
	char longOption[LONG_OPTION_LENGTH];
} OptionPairs;

typedef struct _Parameters {
	char systemFile[FILENAME_MAX];
	char parameterFile[FILENAME_MAX];
} Parameters;

OptionCode getShortOption(char option[]);

OptionCode getLongOption(char option[]);

void getParameters(OptionCode option, char parameter[], void *parameters);

bool getArguments(ushort moreArguments, char *arguments[], Parameters *parameters);

#endif /* ARGUMENTS_H_ */
