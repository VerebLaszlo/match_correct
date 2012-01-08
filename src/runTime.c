/**
 * @file runTime.c
 * @author vereb
 * @date Dec 9, 2011
 * @brief 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "util.h"
#include "runTime.h"

static struct RunTimeCalculator {
	time_t start;
	size_t numberOfRuns;
	size_t numberOfRunsBetweenSteps;
} runTimeCalculator;

void initializeRunTimeCalculator(size_t numberOfRuns, size_t numberOfRunsBetweenSteps) {
	memset(&runTimeCalculator, 0, sizeof(runTimeCalculator));
	runTimeCalculator.numberOfRuns = numberOfRuns;
	runTimeCalculator.numberOfRunsBetweenSteps = numberOfRunsBetweenSteps;
	runTimeCalculator.start = time(NULL);
}

typedef struct {
	size_t second;
	size_t minute;
	size_t hour;
} TimeComponent;

static void convertFromDoubleTo(TimeComponent *timeComponent, double seconds) {
	timeComponent->second = (size_t) seconds;	// all seconds
	timeComponent->minute = timeComponent->second / 60;	// all minutes
	timeComponent->second = timeComponent->second % 60;	// just seconds
	timeComponent->hour = timeComponent->minute / 60;	// just hours
	timeComponent->minute = timeComponent->minute % 60;	// just minutes
}

static void printRemainingTimeAt(size_t current) {
	time_t now = time(NULL);
	double elapsedSeconds = difftime(now, runTimeCalculator.start);
//	double step = elapsedSeconds / (double) current;
//	double wholeSeconds = step * runTimeCalculator.numberOfRuns;
//	TimeComponent whole, remaining;
//	convertFromDoubleTo(&whole, wholeSeconds);
//	convertFromDoubleTo(&remaining, wholeSeconds - elapsedSeconds);
//	printf("%d: Remains \e[0;36m%dh %dm %ds\e[0m of \e[0;31m%dh %dm %ds\e[0m.\n", current,
//		remaining.hour, remaining.minute, remaining.second, whole.hour, whole.minute, whole.second);
	printf("%6d %6g\n", current, elapsedSeconds);
}

void printRemainingTime(size_t current) {
	current++;
	if (!((current) % runTimeCalculator.numberOfRunsBetweenSteps)) {
		printRemainingTimeAt(current);
	}// else if (current <= runTimeCalculator.numberOfRunsBetweenSteps
//		&& !((current) % (runTimeCalculator.numberOfRunsBetweenSteps / 10))) {
//		printRemainingTimeAt(current);
//	}
}
