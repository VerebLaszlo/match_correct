#include "variables.h"
#include "complex.h"

void devideComplexWithDoubleTEST(void) {
	complex numerator, result;
	int min = 0;
	int max = 5;
	int i, j, k;
	for (i = min; i < max; i++) {
		numerator[0] = i;
		for (j = min; j < max; j++) {
			numerator[1] = j;
			for (k = min + 1; k < max; k++) {
				devideComplexWithDouble(numerator, (double) k, result);
				//DevideComplexWithDouble(numerator, (double) k, result);
				printf("("PREC" + "PREC"i) / "PREC" = "PREC" + "PREC"i\n", numerator[0], numerator[1], (double) k,
						result[0], result[1]);
			}
		}
	}
}

void complexConjugateTEST(void) {
	complex num, res;
	num[0] = 1.3234;
	num[1] = 2353.345;
	complexConjugate(num, res);
	printf(PREC" + "PREC"i -> "PREC" + "PREC"i\n", num[0], num[1], res[0], res[1]);
	ComplexConjugate(num, res);
	printf(PREC" + "PREC"i -> "PREC" + "PREC"i\n", num[0], num[1], res[0], res[1]);
}

void productComplexTEST(void) {
	complex left, right, result;
	int min = 1;
	int max = 3;
	int i, j, k, m;
	for (i = min; i < max; i++) {
		left[0] = i;
		for (j = min; j < max; j++) {
			left[1] = j;
			for (k = min; k < max; k++) {
				right[0] = k;
				for (m = min; m < max; m++) {
					right[1] = m;
					//productComplex(left, right, result);
					ProductComplex(left, right, result);
					printf("("PREC" + "PREC"i) * ("PREC" + "PREC"i) = "PREC" + "PREC"i\n", left[0], left[1], right[0],
							right[1], result[0], result[1]);
				}
			}
		}
	}
}

void complexConjugateProductTEST(void) {
	complex left, right, result;
	int min = 1;
	int max = 3;
	int i, j, k, m;
	for (i = min; i < max; i++) {
		left[0] = i;
		for (j = min; j < max; j++) {
			left[1] = j;
			for (k = min; k < max; k++) {
				right[0] = k;
				for (m = min; m < max; m++) {
					right[1] = m;
					complexConjugateProduct(left, right, result);
					//ComplexConjugateProduct(left, right, result);
					printf("("PREC" + "PREC"i) * ("PREC" + "PREC"i) = "PREC" + "PREC"i\n", left[0], left[1],
							right[0], right[1], result[0], result[1]);
				}
			}
		}
	}
}
