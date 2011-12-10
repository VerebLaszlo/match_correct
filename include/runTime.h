/**
 * @file runTime.h
 * @author vereb
 * @date Dec 9, 2011
 * @brief 
 */

#ifndef RUNTIME_H_
#define RUNTIME_H_

/**	Initialize the remaining time calculating object. If you abort the program, call the
 * destroyRunTimeCalculator() function beforehand, this need not to be done if you exit the program.
 * @param[in] numberOfRuns				  : how many times does the code something
 * @param[in] numberOfRunsBetweenMeasures : how many times have to do the code its job
 */
void initializeRunTimeCalculator(size_t numberOfRuns, size_t numberOfRunsBetweenMeasures);

/**	Destroys the remaining time calculatign object. It is called at normal exit.
 */
void destroyRunTimeCalculator(void);

/**	Prints the remaining time according to the initialization.
 * @param file
 * @param currentRun
 */
void printRemainingTime(size_t currentRun);

#endif /* RUNTIME_H_ */
