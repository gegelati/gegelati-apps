/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>
#include <cfloat>
#include <inttypes.h>

#include <gegelati.h>
#include "resultTester.h"

#include "stickGameAdversarial.h"

void agentTest(char* tpgPath) {
	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](int a, int b)->double {return (double)a - (double)b; };
	auto cast = [](int a)->double {return (double)a; };
	auto add = [](double a, double b)->double {return a + b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto nulltest = [](double a)->double {return (a == 0.0) ? 10.0 : 0.0; };
	auto modulo = [](double a, double b)->double {
		if (b != 0.0) { return fmod(a, b); }
		else { return  DBL_MIN; }	};

	set.add(*(new Instructions::LambdaInstruction<double, double>(modulo)));
	set.add(*(new Instructions::LambdaInstruction<int, int>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<int>(cast)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);


	// Instantiate the LearningEnvironment
	auto le = StickGameAdversarial(false);

	// Instantiate the environment that will embed the LearningEnvironment
	Environment env(set, params, le.getDataSources());

	// Instantiate the TPGGraph that we will load
	auto tpg = TPG::TPGGraph(env);

	// Instantiate the tee that will handle the decisions taken by the TPG
	TPG::TPGExecutionEngine tee(env);

	// Create an importer for the best graph and imports it
	File::TPGGraphDotImporter dotImporter(tpgPath, env, tpg);
	dotImporter.importGraph();

	// takes the first root of the graph, anyway out_best has only 1 root (the best)
	auto root = tpg.getRootVertices().back();
	//auto root = tpg.getRootVertices().back();


	size_t x = 0;
	std::cout << "Game :\n" << le.toString() << std::endl;
	// let's play, the only way to leave this loop is to enter -1
	while (x != -1) {
		// gets the action the TPG would decide in this situation
		uint64_t action = (uint64_t)((const TPG::TPGAction*)tee.executeFromRoot(*root).first.back())->getActionID();
		std::cout << "TPG : " << action << std::endl;
		le.doAction((double)action);

		// prints the game board
		std::cout << "Game :\n" << le.toString() << std::endl;

		if (le.isTerminal()) {
			std::cout << "Player won !" << std::endl;
			std::cout << "Reseting game..." << std::endl;
			le.reset();
			continue;
		}

		// gets the action of the player, and checks that he did something legal as he has less discipline than a TPG
		std::cout << "Player move ? " << std::endl;
		std::cin >> x;
		std::cout << x << " chosen" << std::endl;
		if (x < 0 || x>2) {
			continue;
		}
		le.doAction((double)x);


		// prints the game board
		std::cout << "Game :\n" << le.toString() << std::endl;

		if (le.isTerminal()) {
			std::cout << "TPG won !" << std::endl;
			std::cout << "Reseting game..." << std::endl;
			le.reset();
			continue;
		}




	}


	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}
}
