/**
 * @file runTime.c
 * @author vereb
 * @date Dec 9, 2011
 * @brief 
 */

#include <string.h>
#include <time.h>
#include "util.h"
#include "runTime.h"

static struct RunTimeCalculator {
	static size_t numberOfRuns;
	static size_t numberOfRunsBetweenMeasures;
	static size_t numberOfMeasurePoints;
	static size_t numberOfMeasuredPoints;
	static size_t numberOfMeasuredTimeDifferences;
	time_t *timeAt;
	double timeDifference;
	double wholeRunTime;
	double currentRunTime;
} runTimeCalculator;

void initializeRunTimeCalculator(size_t numberOfRuns, size_t numberOfRunsBetweenMeasures) {
	memset(&runTimeCalculator, 0, sizeof(runTimeCalculator));
	runTimeCalculator.numberOfMeasuredTimeDifferences = -1;
	runTimeCalculator.numberOfRuns = numberOfRuns;
	runTimeCalculator.numberOfRunsBetweenMeasures = numberOfRunsBetweenMeasures;
	runTimeCalculator.numberOfMeasurePoints = numberOfRuns / numberOfRunsBetweenMeasures;
	runTimeCalculator.timeAt = secureCalloc(runTimeCalculator.numberOfMeasurePoints + 1,
		sizeof(time_t));
	runTimeCalculator.timeAt[runTimeCalculator.numberOfMeasuredPoints] = time(NULL);
	runTimeCalculator.numberOfMeasuredPoints++;
	runTimeCalculator.numberOfMeasuredTimeDifferences++;
	atexit(destroyRunTimeCalculator());
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
	ushort second;
	ushort minute;
	ushort hour;
} TimeComponent;

static void convertFromDoubleTo(TimeComponent *timeComponent, double seconds) {
	timeComponent->hour = (ushort) seconds % 24;
	timeComponent->minute = (seconds - 24.0 * timeComponent->hour) % 60;
	timeComponent->second = (seconds - 24.0 * timeComponent->hour - 60.0 * timeComponent->minute)
		% 60;
}

static void printRemainingTimeTo(FILE *file) {
	runTimeCalculator.timeAt[runTimeCalculator.numberOfMeasuredPoints] = time(NULL);
	runTimeCalculator.numberOfMeasuredPoints++;
	runTimeCalculator.numberOfMeasuredTimeDifferences++;
	averageTimeDifference();
	runTimeCalculator.wholeRunTime = (double) runTimeCalculator.numberOfMeasurePoints
		* runTimeCalculator.timeDifference;
	runTimeCalculator.currentRunTime = (double) runTimeCalculator.numberOfMeasuredPoints
		* runTimeCalculator.timeDifference;
	TimeComponent difference, whole;
	convertFromDoubleTo(&difference,
		runTimeCalculator.wholeRunTime - runTimeCalculator.currentRunTime);
	convertFromDoubleTo(&whole, runTimeCalculator.wholeRunTime);
	fprintf(file, "The remaining time is %dh %dm %s of %dh %dm %ds\n.", difference.hour,
		difference.minute, difference.second, whole.hour, whole.minute, whole.second);
}

void printRemainingTime(size_t currentRun) {
	if ((currentRun + 1) % runTimeCalculator.numberOfRunsBetweenMeasures == 0) {
		printRemainingTimeTo(stdout);
	}
}
