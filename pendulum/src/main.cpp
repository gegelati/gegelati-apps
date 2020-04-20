#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#include "pendulum.h"
extern "C" {
#include "render.h"
}

#ifndef NB_GENERATIONS
#define NB_GENERATIONS 1200
#endif

void getKey(std::atomic<bool>& exit, std::atomic<bool>& printStats) {
	std::cout << std::endl;
	std::cout << "Press `q` then [Enter] to exit." << std::endl;
	//std::cout << "Press `p` then [Enter] to print classification statistics of the best root." << std::endl;
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
		default:
			printf("Invalid key '%c' pressed.", c);
			std::cout.flush();
		}
	}

	printf("Program will terminate at the end of next generation.\n");
	std::cout.flush();
}

int main() {

	std::cout << "Start Pendulum application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](double a, double b)->double {return a - b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto mult = [](double a, double b)->double {return a * b; };
	auto div = [](double a, double b)->double {return a / b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto ln = [](double a, double b)->double {return std::log(a); };
	auto exp = [](double a, double b)->double {return std::exp(a); };

	set.add(*(new Instructions::LambdaInstruction<double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double>(mult)));
	set.add(*(new Instructions::LambdaInstruction<double>(div)));
	set.add(*(new Instructions::LambdaInstruction<double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(exp)));
	set.add(*(new Instructions::LambdaInstruction<double>(ln)));
	set.add(*(new Instructions::MultByConstParam<double, float>()));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	Learn::LearningParameters params;
	params.mutation.tpg.maxInitOutgoingEdges = 3;
	params.mutation.tpg.nbRoots = 100;
	params.mutation.tpg.pEdgeDeletion = 0.7;
	params.mutation.tpg.pEdgeAddition = 0.7;
	params.mutation.tpg.pProgramMutation = 0.2;
	params.mutation.tpg.pEdgeDestinationChange = 0.1;
	params.mutation.tpg.pEdgeDestinationIsAction = 0.5;
	params.mutation.tpg.maxOutgoingEdges = 5;
	params.mutation.prog.pAdd = 0.5;
	params.mutation.prog.pDelete = 0.5;
	params.mutation.prog.pMutate = 1.0;
	params.mutation.prog.pSwap = 1.0;
	params.mutation.prog.maxProgramSize = 20;
	params.maxNbActionsPerEval = 1500;
	params.nbIterationsPerPolicyEvaluation = 5;
	params.ratioDeletedRoots = 0.90;
	params.archiveSize = 500;
	params.archivingProbability = 0.01;

	// Instantiate the LearningEnvironment
	Pendulum pendulumLE({ 0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0 });

	std::cout << "Number of threads: " << std::thread::hardware_concurrency() << std::endl;

	// Instantiate and init the learning agent
	Learn::LearningAgent la(pendulumLE, set, params);
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

	// Train for NB_GENERATIONS generations
	printf("\nGen\tNbVert\tMin\tAvg\tMax\tTvalid\tTtrain\n");
	for (int i = 0; i < NB_GENERATIONS && !exitProgram; i++) {
		char buff[12];
		sprintf(buff, "out_%04d.dot", i);
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

		if (printStats || i % 10 == 0) {
			//mnistLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), iter->second);
			//printStats = false;
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

	// Print stats one last time
	//mnistLE.printClassifStatsTable(la.getTPGGraph().getEnvironment(), la.getTPGGraph().getRootVertices().at(0));

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