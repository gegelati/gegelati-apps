#include <instructions/lambdaInstruction.h>

#include <iostream>

int main() {
	std::cout << "Hello" << std::endl;

	Instructions::LambdaInstruction<int> addition([](int a, int b) {return a + b; });

	return 0;
}