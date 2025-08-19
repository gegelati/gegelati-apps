#include <iostream>
#include <cfloat>

#include <gegelati.h>

#include "../Learn/instructions.h"

int main() {

	Instructions::Set set;
	fillInstructionSet(set);

	std::cout << set.getInstruction(3).getNbOperands() << std::endl;
	Data::PrimitiveTypeArray<int> remainingSticks(1);
	Data::PrimitiveTypeArray<int> hints(4);

	std::string path(ROOT_DIR "/src/CodeGen/");

	std::vector<std::reference_wrapper<const Data::DataHandler>> data = { hints, remainingSticks };
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
	Environment dotEnv(set, params, data);

	TPG::TPGGraph dotGraph(dotEnv);
	std::string filename(path + "StickGame_out_best.dot");

	File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
	dot.importGraph();

	CodeGen::TPGGenerationEngineFactory factory(CodeGen::TPGGenerationEngineFactory::switchMode);
	std::unique_ptr<CodeGen::TPGGenerationEngine> tpggen = factory.create("stickgame", dotGraph, "src/");
	tpggen->generateTPGGraph();

	return 0;

}

