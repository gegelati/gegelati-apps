#include <iostream>
#include <cfloat>
#include <ctime>

#include <gegelati.h>

/// instantiate global variable used to communicate between the TPG and the environment
int* in1;
int* in2;

#include "../Learn/instructions.h"
#include "../Learn/stickGameAdversarial.h"

int main() {

	/// Import instruction set used during training(required only for gegelati Inference)
	Instructions::Set set;
	fillInstructionSet(set);

	/// initialise AdversarialLearningEnvironment
	auto le = StickGameAdversarial(false);

	/// Instantiate an Environment and import (required only for gegelati Inference)
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);

	// Load graph from dot file
	std::string path(ROOT_DIR "/src/CodeGen/");
	Environment dotEnv(set, le.getDataSources(), params.nbRegisters, params.nbProgramConstant);
	TPG::TPGGraph dotGraph(dotEnv);
	std::string filename(path + "StickGame_out_best.dot");
	File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
	dot.importGraph();

	// Prepare for inference
	TPG::TPGExecutionEngine tee(dotEnv);
	const TPG::TPGVertex* root(dotGraph.getRootVertices().back());

	/// fetch data in the environment
	auto& st = le.getDataSources().at(0).get();
	in1 = st.getDataAt(typeid(int), 0).getSharedPointer<int>().get();
	auto& st2 = le.getDataSources().at(1).get();
	in2 = st2.getDataAt(typeid(int), 0).getSharedPointer<int>().get();

	/// set the number of game
	int playerNb = 0;
	const size_t nbGames = 100;
	std::cout << "Game : " << le.toString() << std::endl;

	// let's play, the only way to leave this loop is to play nbGames games
	int nbActions = 0;
	size_t actions[1000];
	// measure time
	auto start = std::chrono::system_clock::now();

	size_t playedGames = 0;
	while (playedGames < nbGames) {

		///inference with generated C files
		auto trace = tee.executeFromRoot(*root);
		actions[nbActions] = ((const TPG::TPGAction*)trace.back())->getActionID();

		std::cout << "player : " << playerNb << " removes : " << actions[nbActions] + 1 << " sticks " << std::endl;
		le.doAction(actions[nbActions]);
		playerNb = !playerNb;

		nbActions++;

		if (le.isTerminal()) {
			std::cout << "TPG nb" << playerNb << " won !" << std::endl;
			le.reset();
			++playedGames;
			continue;
		}
	}

	auto stop = std::chrono::system_clock::now();

	// do a replay to subtract non-inference time
	playedGames = 0;
	nbActions = 0;

	std::cout << "Replay environment without TPG" << std::endl;
	auto startReplay = std::chrono::system_clock::now();
	while (playedGames < nbGames) {

		std::cout << "player : " << playerNb << " removes : " << actions[nbActions] + 1 << " sticks " << std::endl;
		le.doAction(actions[nbActions]);
		playerNb = !playerNb;

		nbActions++;

		if (le.isTerminal()) {
			std::cout << "TPG nb" << playerNb << " won !" << std::endl;
			le.reset();
			++playedGames;
			continue;
		}
	}

	auto stopReplay = std::chrono::system_clock::now();
	auto totalTime = ((std::chrono::duration<double>)(stop - start)).count();
	auto replayTime = ((std::chrono::duration<double>)(stopReplay - startReplay)).count();
	std::cout << std::setprecision(6) << " Total time: " << totalTime << std::endl;
	std::cout << std::setprecision(6) << "  Env. time: " << replayTime << std::endl;
	std::cout << std::setprecision(6) << "Infer. time: " << totalTime - replayTime << std::endl;

}