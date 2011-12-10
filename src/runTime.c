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
	size_t numberOfRuns;
	size_t numberOfRunsBetweenMeasures;
	size_t numberOfMeasurePoints;
	size_t numberOfMeasuredPoints;
	size_t numberOfMeasuredTimeDifferences;
	time_t *timeAt;
	double timeDifference;
	double wholeRunTime;
	double currentRunTime;
} runTimeCalculator;

void initializeRunTimeCalculator(size_t numberOfRuns, size_t numberOfRunsBetweenMeasures) {
	memset(&runTimeCalculator, 0, sizeof(runTimeCalculator));
	runTimeCalculator.numberOfRuns = numberOfRuns;
	runTimeCalculator.numberOfRunsBetweenMeasures = numberOfRunsBetweenMeasures;
	runTimeCalculator.numberOfMeasurePoints = numberOfRuns / numberOfRunsBetweenMeasures;
	runTimeCalculator.timeAt = secureCalloc(runTimeCalculator.numberOfMeasurePoints * 2,
		sizeof(time_t));
	runTimeCalculator.timeAt[runTimeCalculator.numberOfMeasuredPoints] = time(NULL);
	runTimeCalculator.numberOfMeasuredPoints++;
	atexit(destroyRunTimeCalculator);
}

void destroyRunTimeCalculator(void) {
	secureFree(runTimeCalculator.timeAt);
}

static void averageTimeDifference(void) {
	double timeSum = 0.0;
	for (size_t current = 0; current < runTimeCalculator.numberOfMeasuredPoints - 1; current++) {
		timeSum += difftime(runTimeCalculator.timeAt[current + 1],
			runTimeCalculator.timeAt[current]);
	}
	runTimeCalculator.timeDifference = timeSum
		/ (double) runTimeCalculator.numberOfMeasuredTimeDifferences;
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

static void printRemainingTimeTo(void) {
	runTimeCalculator.timeAt[runTimeCalculator.numberOfMeasuredPoints] = time(NULL);
	runTimeCalculator.numberOfMeasuredPoints++;
	runTimeCalculator.numberOfMeasuredTimeDifferences++;
	averageTimeDifference();
	runTimeCalculator.wholeRunTime = ((double) runTimeCalculator.numberOfMeasurePoints + 1.0)
		* runTimeCalculator.timeDifference;
	runTimeCalculator.currentRunTime = (double) runTimeCalculator.numberOfMeasuredTimeDifferences
		* runTimeCalculator.timeDifference;
	TimeComponent difference, whole;
	convertFromDoubleTo(&difference,
		runTimeCalculator.wholeRunTime - runTimeCalculator.currentRunTime);
	convertFromDoubleTo(&whole, runTimeCalculator.wholeRunTime);
	printf("The remaining time is \e[0;36m%dh %dm %ds\e[0m of \e[0;31m%dh %dm %ds\e[0m.\n",
		difference.hour, difference.minute, difference.second, whole.hour, whole.minute,
		whole.second);
}

void printRemainingTime(size_t currentRun) {
	if ((currentRun + 1) % runTimeCalculator.numberOfRunsBetweenMeasures == 0) {
		printf("%10d: ", currentRun + 1);
		printRemainingTimeTo();
	}
}
