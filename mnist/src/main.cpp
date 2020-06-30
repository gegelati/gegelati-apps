#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#include <gegelati.h>

#include "mnist.h"

#ifndef NB_GENERATIONS
#define NB_GENERATIONS 300
#endif


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

	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(exp)));
	set.add(*(new Instructions::LambdaInstruction<double>(ln)));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json",params);

	// Instantiate the LearningEnvironment
	MNIST mnistLE;

	std::cout << "Number of threads: " << std::thread::hardware_concurrency() << std::endl;

	// Instantiate and init the learning agent
	Learn::ClassificationLearningAgent la(mnistLE, set, params);
	la.init();

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_000.dot", la.getTPGGraph());


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

	// File for printing best policy stat.
	std::ofstream stats;
	stats.open("bestPolicyStats.md");
	const TPG::TPGVertex* bestRoot = nullptr;

	// Train for NB_GENERATIONS generations
	printf("\nGen\tNbVert\tMin\tAvg\tMax\tTvalid\tTtrain\n");
	for (int i = 0; i < NB_GENERATIONS && !exitProgram; i++) {
		char buff[12];
		sprintf(buff, "out_%03d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();
		std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> result;
		auto startEval = std::chrono::high_resolution_clock::now();
		result = la.evaluateAllRoots(i, Learn::LearningMode::VALIDATION);
		auto stopEval = std::chrono::high_resolution_clock::now();
		auto iter = result.begin();
		double min = iter->first->getResult();
		std::advance(iter, result.size() - 1);
		double max = iter->first->getResult();
		double avg = std::accumulate(result.begin(), result.end(), 0.0,
			[](double acc, std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> pair)->double {return acc + pair.first->getResult(); });
		avg /= result.size();
		printf("%3d\t%4" PRIu64 "\t%1.2lf\t%1.2lf\t%1.2lf", i, la.getTPGGraph().getNbVertices(), min, avg, max);
		std::cout << "\t" << std::chrono::duration_cast<std::chrono::milliseconds>(stopEval - startEval).count();

		// Print stats in file if a new best root was found

		if (la.getBestRoot().first != bestRoot) {
			bestRoot = la.getBestRoot().first;
			stats << "Generation " << i << std::endl << std::endl;
			TPG::PolicyStats ps;
			ps.setEnvironment(la.getTPGGraph().getEnvironment());
			ps.analyzePolicy(bestRoot);
			stats << ps << std::endl;
			stats << std::endl << std::endl << "==========" << std::endl << std::endl;
		}

		if (printStats) {
			mnistLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), iter->second);
			printStats = false;
		}
		std::cout.flush();

		auto startTrain = std::chrono::high_resolution_clock::now();
		la.trainOneGeneration(i);
		auto stopTrain = std::chrono::high_resolution_clock::now();

		std::cout << "\t" << std::chrono::duration_cast<std::chrono::milliseconds>(stopTrain - startTrain).count() << std::endl;
	}

	// Keep best policy
	la.keepBestPolicy();
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();

	TPG::PolicyStats ps;
	ps.setEnvironment(la.getTPGGraph().getEnvironment());
	ps.analyzePolicy(la.getBestRoot().first);
	std::ofstream bestStats;
	bestStats.open("out_best_stats.md");
	bestStats << ps;
	bestStats.close();
	stats.close();

	// Print stats one last time
	mnistLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), la.getTPGGraph().getRootVertices().at(0));

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
