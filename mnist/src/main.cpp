#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#include <gegelati.h>

#include "mnist.h"

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
	std::cout << "Start MNIST application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](double a, double b)->double {return a - b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto mult = [](double a, double b)->double {return a * b; };
	auto div = [](double a, double b)->double {return a / b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto ln = [](double a)->double {return std::log(a); };
	auto exp = [](double a)->double {return std::exp(a); };
	auto multByConst = [](double a, Data::Constant c)->double {return a * (double)c / 10.0; };
	auto sobelMagn = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = sqrt(gx * gx + gy * gy);
		return result;
	};

	auto sobelDir = [](const double a[3][3])->double {
		double result = 0.0;
		double gx =
			-a[0][0] + a[0][2]
			- 2.0 * a[1][0] + 2.0 * a[1][2]
			- a[2][0] + a[2][2];
		double gy = -a[0][0] - 2.0 * a[0][1] - a[0][2]
			+ a[2][0] + 2.0 * a[2][1] + a[2][2];
		result = std::atan(gy / gx);
		return result;
	};

	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(exp)));
	set.add(*(new Instructions::LambdaInstruction<double>(ln)));
	set.add(*(new Instructions::LambdaInstruction<double, Data::Constant>(multByConst)));
	set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelMagn)));
	set.add(*(new Instructions::LambdaInstruction<const double[3][3]>(sobelDir)));

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
	MNIST mnistLE;

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

	// Instantiate and init the learning agent
	Learn::ClassificationLearningAgent la(mnistLE, set, params);
	la.init();

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_0000.dot", *la.getTPGGraph());

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
		char buff[13];
		sprintf(buff, "out_%04d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();

		la.trainOneGeneration(i);

		if (printStats) {
			mnistLE.printClassifStatsTable(la.getTPGGraph()->getEnvironment(), la.getBestRoot().first);
			printStats = false;
		}
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

	// close log file also
	stats.close();

	// Print stats one last time
	mnistLE.printClassifStatsTable(la.getTPGGraph()->getEnvironment(), la.getTPGGraph()->getRootVertices().at(0));

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
