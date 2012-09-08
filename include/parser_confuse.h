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
	BH = 2,	///< number of blackholes in the binary system.
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
	char approximant[STRING_LENGTH];	///< approximant to use.
	char spin[STRING_LENGTH];	///< spin contribution to use.
	int phase;	///< double of the PN order in phase.
	int amplitude;	///< double of the PN order in amplitude.
} Method;

/** Waveform parameters. */
typedef struct {
	Binary binary;	///< parameters of the binary emitting the waveform.
	Method method;	///< waveform generating method.
	size_t number;	///< number of the runs.
	char name[STRING_LENGTH];	///< name of the generated waveform.
} Wave;

/** Parameters to generate waveforms. */
typedef struct {
	double initialFrequency;	///< initial frequency.
	double endingFrequency;	///< ending frequency.
	double samplingFrequency;	///< sampling frequency.
	double samplingTime;	///< sampling time.
	size_t length;	///< number of waveform pairs.
	Wave *wave;	///< default wave parameters.
	string *name;	///< name of the waveform pairs.
} Parameter;

/**
 * Initialise the parser.
 */
void initParser(void);

int parseWaves(char *file, Parameter *parameter);

/**
 * Parse the config file.
 * @param[in] file name of the configuratin file
 * @return error code
 */
int parse(char *file, Parameter *parameters);

int printWaveParameter(FILE *file, Wave *wave);

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
