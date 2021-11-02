#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#include <gegelati.h>

#include "cifar.h"

void getKey(std::atomic<bool>& exit, std::atomic<bool>& printStats) {
	std::cout << std::endl;
	std::cout << "Press `q` then [Enter] to exit." << std::endl;
	std::cout << "Press `p` then [Enter] to print classification statistics of the best root." << std::endl;
	std::cout.flush();

	exit = false;

	while (!exit) {
		char c;
		std::cin >> c;
		switch (c) {
		case 'q':
		case 'Q':
			exit = true;
			break;
		case 'p':
		case 'P':
			printStats = true;
			break;
		default:
			printf("Invalid key '%c' pressed.", c);
			std::cout.flush();
		}
	}

	printf("Program will terminate at the end of next generation.\n");
	std::cout.flush();
}

int main() {
	std::cout << "Start CIFAR application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](double a, double b)->double {int64_t tmpres = *(int64_t*)&a - *(int64_t*)&b; return *(double*)&(tmpres); };
	auto add = [](double a, double b)->double {int64_t tmpres = *(int64_t*)&a + *(int64_t*)&b; return *(double*)&(tmpres); };
	auto mult = [](double a)->double {int64_t tmpres = *(int64_t*)&a*2; return *(double*)&(tmpres); };
	auto div = [](double a)->double {int64_t tmpres = *(int64_t*)&a/2; return *(double*)&(tmpres); };
	auto cond = [] (double a, double b)->double {int64_t tmpa = *(int64_t*)&a; int64_t tmpb = *(int64_t*)&b; return tmpa < tmpb ? a : *(double*)&(tmpa);};

	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(cond)));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
#ifdef NB_GENERATIONS
	params.nbGenerations = NB_GENERATIONS;
#endif // !NB_GENERATIONS

	// Instantiate the LearningEnvironment
	CIFAR cifarLE;

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

	// Instantiate and init the learning agent
	Learn::ParallelLearningAgent la(cifarLE, set, params);
	la.init();

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_0000.dot", la.getTPGGraph());

	// Start a thread for controlling the loop
#ifndef NO_CONSOLE_CONTROL
	std::atomic<bool> exitProgram = true; // (set to false by other thread) 
	std::atomic<bool> printStats = false;

	std::thread threadKeyboard(getKey, std::ref(exitProgram), std::ref(printStats));

	while (exitProgram); // Wait for other thread to print key info.
#else 
	std::atomic<bool> exitProgram = false; // (set to false by other thread) 
	std::atomic<bool> printStats = false;
#endif

	// Adds a logger to the LA (to get statistics on learning) on std::cout
	Log::LABasicLogger logCout(la);

	// File for printing best policy stat.
	std::ofstream stats;
	stats.open("bestPolicyStats.md");
	Log::LAPolicyStatsLogger logStats(la, stats);

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	File::ParametersParser::writeParametersToJson("exported_params.json", params);

	// Train for NB_GENERATIONS generations
	for (int i = 0; i < params.nbGenerations && !exitProgram; i++) {
		char buff[20];
		sprintf(buff, "out_%05d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();
		la.trainOneGeneration(i);

		if (printStats) {
			cifarLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), la.getBestRoot().first);
			printStats = false;
		}
		cifarLE.updateSubset();
	}

	// Keep best policy
	la.keepBestPolicy();

	// Clear introns instructions
	la.getTPGGraph().clearProgramIntrons();

	// Export the graph
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();

	TPG::PolicyStats ps;
	ps.setEnvironment(la.getTPGGraph().getEnvironment());
	ps.analyzePolicy(la.getBestRoot().first);
	std::ofstream bestStats;
	bestStats.open("out_best_stats.md");
	bestStats << ps;
	bestStats.close();

	// close log file also
	stats.close();

	// Print stats one last time
	cifarLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), la.getTPGGraph().getRootVertices().at(0));

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

#ifndef NO_CONSOLE_CONTROL
	// Exit the thread
	std::cout << "Exiting program, press a key then [enter] to exit if nothing happens.";
	threadKeyboard.join();
#endif

	return 0;
}
