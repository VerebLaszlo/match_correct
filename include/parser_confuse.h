/**	@file	parser_confuse.h
 *	@author vereb
 *	@date	29 Aug 2012
 *	@brief
 */

#ifndef PARSER_CONFUSE_H_
#define PARSER_CONFUSE_H_

#include "util.h"

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
	double inclination[BH];	///< inclination of the spin \f$[0,\pi]\rad\f$.
	double azimuth[BH];	///< azimuth of the spin \f$[0,2\pi)\rad\f$.
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
	Wave defaultWave;	///< default wave parameters.
} Parameter;

/**
 * Parse the config file.
 * @param[in] file name of the configuratin file
 * @return error code
 */
int parse(char *file, Parameter *parameters);

int printParameter(FILE *file, Parameter *parameter);

#endif /* PARSER_CONFUSE_H_ */
