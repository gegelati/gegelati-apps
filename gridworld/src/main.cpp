#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>
#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "gridworld.h"
#include "instructions.h"

int main() {

	std::cout << "Start GridWorld application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	fillInstructionSet(set);

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);

	// Instantiate the LearningEnvironment
	GridWorld gridWorldLe;

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

	
	// Instantiate and init the learning agent
	Learn::ParallelLearningAgent la(gridWorldLe, set, params);
	la.init();

	const TPG::TPGVertex* bestRoot = NULL;

	// Basic logger
	Log::LABasicLogger basicLogger(la);

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_0000.dot", *la.getTPGGraph());

	// Logging best policy stat.
	std::ofstream stats;
	stats.open("bestPolicyStats.md");
	Log::LAPolicyStatsLogger policyStatsLogger(la, stats);

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	File::ParametersParser::writeParametersToJson("exported_params.json", params);

	// Train for params.nbGenerations generations
	for (int i = 0; i < params.nbGenerations; i++) {
		char buff[13];
		sprintf(buff, "out_%04d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();

		la.trainOneGeneration(i);

	}

	// Keep best policy
	la.keepBestPolicy();

	// Clear introns instructions
	la.getTPGGraph()->clearProgramIntrons();

	// Export the graph
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();

	TPG::PolicyStats ps;
	ps.setEnvironment(la.getTPGGraph()->getEnvironment());
	ps.analyzePolicy(la.getBestRoot().first);
	std::ofstream bestStats;
	bestStats.open("out_best_stats.md");
	bestStats << ps;
	bestStats.close();
	stats.close();

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}