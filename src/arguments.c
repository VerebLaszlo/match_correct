/**
 * @file arguments.c
 * @author vereb
 * @date Aug 10, 2011
 * @brief 
 */

#include "arguments.h"
#include <string.h>

OptionPairs params[] = { { NONE, "", "" }, { HELP, "-h", "--help" },
	{ SYSTEM_FILE, "-s", "--system" }, { PARAMETER_FILE, "-p", "--parameter" } };

OptionCode getOption(char option[]) {
	OptionCode code = NONE;
	ushort i = 0;
	while (params[i].code < NO_MORE_PARAMETER_CODE - 1) {
		if (!strcmp(params[i].shortOption, option) || !strcmp(params[i].longOption, option)) {
			code = params[i].code;
		}
		i++;
	}
	return code;
}

void getParameters(OptionCode option, char parameter[], void *_parameters) {
	Parameters *parameters = (Parameters*) _parameters;
	switch (option) {
	case SYSTEM_FILE:
		strcpy(parameters->systemFile, parameter);
		break;
	case PARAMETER_FILE:
		strcpy(parameters->parameterFile, parameter);
		break;
	default:
		break;
	}
}

bool getArguments(ushort moreArguments, char *arguments[], Parameters *parameters) {
	bool loadDefaults = false;
	bool returnMode = true;
	if (moreArguments) {
		OptionCode optionFound;
		do {
			optionFound = getOption(*arguments);
			if (optionFound) {
				(*arguments)++;
				moreArguments--;
				getParameters(optionFound, *arguments, parameters);
			} else {
				// Do You want to load the defaults?
				loadDefaults = true;
				if (!loadDefaults) {
					returnMode = false;
				}
			}
		} while (moreArguments--);
	} else {
// Do You want to load the defaults?
		loadDefaults = true;
		if (!loadDefaults) {
			returnMode = false;
		}
	}
	if (loadDefaults) {
		//loadDefaults();
	}
	return returnMode;
}
