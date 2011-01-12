#include "variables.h"
#include "complex.h"

void devideComplexWithDoubleTEST(void);

void conjugateTEST(void);

void conjugateProductTEST(void);

void conjugateProductRealTEST(void);

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
				//devideComplexWithDouble(numerator, (double) k, result);
				DevideComplexWithDouble(numerator, (double) k, result);
				printf("("PREC" + "PREC"i) / "PREC"\n "PREC" + "PREC"i\n",numerator[0], numerator[1], (double)k, result[0], result[1]);
		}
	}
}
}

void conjugateTest(void) {
	complex num, res;
	num[0] = 1.3234;
	num[1] = 2353.345;
	conjugate(num, res);
	printf(PREC" + "PREC"i\n"PREC" + "PREC"i\n", num[0], num[1], res[0], res[1]);
}
