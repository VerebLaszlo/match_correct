/**
 * @file match.h
 * @author László Veréb
 * @date 2010.04.08.
 */

#ifndef MATCH_H
#define MATCH_H

#include "signals.h"

/**
 * Formulae given.
 * \f{gather*}{
 * 	M_{best}=\max_{t_0}\left(\frac{A+B}{2}+\left[\left(\frac{A-B}{2}\right)^2+C^2\right]^{0.5}\right)^{0.5}\;
 * 	M_{minimax}=\max_{t_0}\left(\frac{A+B}{2}-\left[\left(\frac{A-B}{2}\right)^2+C^2\right]^{0.5}\right)^{0.5}\\
 *	A=\inProd{\tilde{e}_{1+}}{\tilde{e}_{2+}}^2+\inProd{\tilde{e}_{1+}}{\tilde{e}_{2\bot}}^2\\
 *	B=\inProd{\tilde{e}_{1\bot}}{\tilde{e}_{2+}}^2+\inProd{\tilde{e}_{1\bot}}{\tilde{e}_{2\bot}}^2\\
 *	C=\inProd{\tilde{e}_{1+}}{\tilde{e}_{2+}}+\inProd{\tilde{e}_{1\bot}}{\tilde{e}_{2+}}+\inProd{\tilde{e}_{1+}}{\tilde{e}_{2\bot}}+\inProd{\tilde{e}_{1\bot}}{\tilde{e}_{2\bot}}
 * \f}
 * @param in
 * @param min_Index
 * @param max_Index
 * @param typ
 * @param best
 * @param minimax
 */
void calc_Matches(SignalStruct *in, size_t min_Index, size_t max_Index, double *typ, double *best,
	double *minimax);

/**	Calculates the index boundaries from the frequency boundaries.
 * @param[in] min			 : starting frequency
 * @param[in] max			 : ending frequency
 * @param[in] step			 : sampling frequency
 * @param[out] startingIndex : stariting index
 * @param[out] endingIndex	 : ending index
 */
void calculateIndexBoundariesFromFrequencies(double min, double max, double step,
	size_t *startingIndex, size_t *endingIndex);

void countPeriods(SignalStruct *signal, size_t periods[]);

#endif
