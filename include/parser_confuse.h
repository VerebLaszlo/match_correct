/**	@file   parser_confuse.h
 *	@author László Veréb
 *	@date   29 Aug 2012
 *	@brief  Configuration file parser.
 */

#ifndef PARSER_CONFUSE_H_
#define PARSER_CONFUSE_H_

#include "util.h"
#include "util_math.h"

typedef char string[STRING_LENGTH];

/** Parameter specific constants. */
enum {
	FIRST, SECOND, THIRD, BH = THIRD,	///< number of blackholes in the binary system.
};

/** Coordinate system conventions. */
typedef enum {
	PRECESSING, FIXED,
} CoordinateSystem;

/** Spin parameters. */
typedef struct {
	double magnitude[BH];	///< magnitude of the spin.
	double inclination[BH];	///< inclination of the spin \f$[0,\pi]\rad\f$ in precessing frame.
	double azimuth[BH];	///< azimuth of the spin \f$[0,2\pi)\rad\f$ in precessing frame.
	double component[BH][DIMENSION];	///< components of the spn in fixed frame.
	CoordinateSystem system;	///< coordinate system convention.
} Spin;

/** Binary parameters. */
typedef struct {
	double mass[BH];	///< masses of the blackholes.
	Spin spin;	///< spins of the blackholes.
	double inclination;	///< inclination of the orbital plane.
	double distance;	///< distance of the source from the detector.
} Binary;

/** Waveform generation method. */
typedef struct {
	char spin[STRING_LENGTH];	///< spin contribution to use.
	int phase;	///< double of the PN order in phase.
	int amplitude;	///< double of the PN order in amplitude.
} Method;

/** Waveform parameters. */
typedef struct {
	Binary binary;	///< parameters of the binary emitting the waveform.
	Method method;	///< waveform generating method.
	size_t number;	///< number of the runs.
	string name;	///< name of the generated waveform.
	double diff[2];
} Wave;

typedef struct {
	size_t length;	///< number of waveform pairs.
	Wave *wave;	///< default wave parameters.
	string *name;	///< name of the waveform pairs.
} WavePair;

/** Parameters to generate waveforms. */
typedef struct {
	double initialFrequency;	///< initial frequency.
	double endingFrequency;	///< ending frequency.
	double samplingFrequency;	///< sampling frequency.
	double samplingTime;	///< sampling time.
	WavePair *exact;
	WavePair *step;
} Parameter;

/**
 * Initialise the parser.
 */
void initParser(void);

int parseWaves(char *file, Parameter *parameter);

void cleanParameter(Parameter *parameter);

/**
 * Prints the paramters.
 * @param[in] file      where to print.
 * @param[in] parameter to print.
 * @param[in] from
 * @param[in] to
 * @return
 */
int printParameter(FILE *file, Parameter *parameter, int from, int to);

#endif /* PARSER_CONFUSE_H_ */
