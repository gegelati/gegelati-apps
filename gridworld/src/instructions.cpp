#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "instructions.h"

void fillInstructionSet(Instructions::Set& set) {
	auto minus = [](double a, double b) -> double { return a - b; };
	auto add = [](double a, double b) -> double { return a + b; };
	auto mult = [](double a, double b) -> double { return a * b; };
	auto div = [](double a, double b) -> double { return a / b; };
	auto max = [](double a, double b) -> double { return std::max(a, b); };
	auto ln = [](double a) -> double { return std::log(a); };
	auto exp = [](double a) -> double { return std::exp(a); };
	auto cos = [](double a) -> double { return std::cos(a); };
	auto sin = [](double a) -> double { return std::sin(a); };
	auto tan = [](double a) -> double { return std::tan(a); };

	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(exp)));
	set.add(*(new Instructions::LambdaInstruction<double>(ln)));
	set.add(*(new Instructions::LambdaInstruction<double>(cos)));
	set.add(*(new Instructions::LambdaInstruction<double>(sin)));
	set.add(*(new Instructions::LambdaInstruction<double>(tan)));
}