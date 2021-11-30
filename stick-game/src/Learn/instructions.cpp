#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "instructions.h"

void fillInstructionSet(Instructions::Set& set) {
	auto minus = [](int a, int b)->double {return (double)a - (double)b; };
	auto cast = [](int a)->double {return (double)a; };
	auto add = [](double a, double b)->double {return a + b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto nulltest = [](double a)->double {return (a == 0.0) ? 10.0 : 0.0; };
	auto modulo = [](double a, double b)->double {
		if (b != 0.0) { return fmod(a, b); }
		else { return  DBL_MIN; }	};

	set.add(*(new Instructions::LambdaInstruction<double, double>(modulo, "$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;")));
	set.add(*(new Instructions::LambdaInstruction<int, int>(minus, "$0 = (double)($1) - (double)($2);")));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add, "$0 = $1 + $2;")));
	set.add(*(new Instructions::LambdaInstruction<int>(cast, "$0 = (double)($1);")));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max, "$0 = (($1) < ($2)) ? ($2) : ($1); ")));
	set.add(*(new Instructions::LambdaInstruction<double>(nulltest, "$0 = ($1 == 0.0) ? 10.0 : 0.0;")));
}