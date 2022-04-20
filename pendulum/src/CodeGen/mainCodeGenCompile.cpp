#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>
#include <iostream>
#include <float.h>

#include <gegelati.h>

#include "../Learn/instructions.h"


int main() {

	std::cout << "Generate C code from pre-trained dot file." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	fillInstructionSet(set);

	std::string path(ROOT_DIR "/src/CodeGen/");

	Data::PrimitiveTypeArray<double> currentState{ 2 };
	std::vector<std::reference_wrapper<const Data::DataHandler>> data = { currentState };

	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(
		ROOT_DIR"/params.json", params);
	Environment dotEnv(set, data, params.nbRegisters, params.nbProgramConstant);
	TPG::TPGGraph dotGraph(dotEnv);

	std::string filename(path + "Pendulum_out_best.dot");
	File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
	dot.importGraph();

	CodeGen::TPGGenerationEngineFactory factory(CodeGen::TPGGenerationEngineFactory::switchMode);
	std::unique_ptr<CodeGen::TPGGenerationEngine> tpggen = factory.create("pendulum", dotGraph, "src/");
	tpggen->generateTPGGraph();


	return 0;
}