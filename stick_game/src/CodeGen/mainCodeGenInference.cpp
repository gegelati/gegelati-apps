#include <iostream>
#include <cfloat>
#include <ctime>

#include <gegelati.h>

extern "C" {
#include "stick_game.h"
	/// instantiate global variable used to communicate between the TPG and the environment
	int* in1;
	int* in2;
}

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
	Environment env(set, le.getDataSources(), params.nbRegisters);

	/// fetch data in the environment
	auto& st = le.getDataSources().at(0).get();
	in1 = st.getDataAt(typeid(int), 0).getSharedPointer<int>().get();
	auto& st2 = le.getDataSources().at(1).get();
	in2 = st2.getDataAt(typeid(int), 0).getSharedPointer<int>().get();

	/// set the number of game
	uint64_t action;
	int playerNb = 0;
	size_t nbParties = 10;
	std::cout << "Game : " << le.toString() << std::endl;
	// let's play, the only way to leave this loop is to play nbParties games
	while (nbParties != 0) {

		///inference with generated C files
		action = inferenceTPG();

		std::cout << "player : " << playerNb << " removes : " << action + 1 << " sticks " << std::endl;
		le.doAction(action);
		playerNb = !playerNb;

		if (le.isTerminal()) {
			std::cout << "TPG nb" << playerNb << " won !" << std::endl;
			le.reset();
			nbParties--;
			continue;
		}
	}

}