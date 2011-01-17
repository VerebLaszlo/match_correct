//#include <stdio.h>

#include "complex_Test.h"
#include "util_math.h"

int main(int argc, char* argv[]) {
	puts("Start.");
	double x = 23.4;
	printf("%lg^2 = %lg = %lg\n",x, x*x, SQR(x));
	//devideComplexWithDoubleTEST();
	//complexConjugateTEST();
	//productComplexTEST();
	//complexConjugateProductTEST();
	puts("Done.");
	return 0;
}
