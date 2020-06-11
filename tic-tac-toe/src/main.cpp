#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>
#include <cfloat>
#include <inttypes.h>

#include <gegelati.h>

#include "TicTacToe.h"
#include "resultTester.cpp"

#ifndef NB_GENERATIONS
#define NB_GENERATIONS 600
#endif


int main() {
	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](double a, double b)->double {return (double)a - (double)b; };
	auto add = [](double a, double b)->double {return a + b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
    auto modulo = [](double a, double b)->double {return b != 0.0 ? fmod(a,b):DBL_MIN;};
	auto nulltest = [](double a)->double {return (a == -1.0) ? 10.0 : 0.0; };
	auto circletest = [](double a)->double {return (a == 0.0) ? 10.0 : 0.0; };
	auto crosstest = [](double a)->double {return (a == 1.0) ? 10.0 : 0.0; };
	auto test15 = [](double a)->double {return (a >= 15.0) ? 10.0 : 0.0; };
    auto cond = [](double a, double b) -> double { return a < b ? -a : a; };


	set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
	set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo)));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));
	set.add(*(new Instructions::LambdaInstruction<double>(circletest)));
	set.add(*(new Instructions::LambdaInstruction<double>(crosstest)));
	set.add(*(new Instructions::LambdaInstruction<double>(test15)));
    set.add(*(new Instructions::LambdaInstruction<double,double>(cond)));


	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	Learn::LearningParameters params;
	params.mutation.tpg.maxInitOutgoingEdges = 3;
	params.mutation.tpg.nbRoots = 200;
	params.mutation.tpg.pEdgeDeletion = 0.7;
	params.mutation.tpg.pEdgeAddition = 0.7;
	params.mutation.tpg.pProgramMutation = 0.7;
	params.mutation.tpg.pEdgeDestinationChange = 0.2;
	params.mutation.tpg.pEdgeDestinationIsAction = 0.6;
	params.mutation.tpg.maxOutgoingEdges = 10;
	params.mutation.prog.pAdd = 0.6;
	params.mutation.prog.pDelete = 0.6;
	params.mutation.prog.pMutate = 1.0;
	params.mutation.prog.pSwap = 1.0;
	params.mutation.prog.maxProgramSize = 70;
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 5;
	params.maxNbEvaluationPerPolicy = 500;
	params.nbIterationsPerPolicyEvaluation = 100;
	params.ratioDeletedRoots = 0.9;

	// Instantiate the LearningEnvironment
	TicTacToe le;

	// Instantiate and init the learning agent
	Learn::ParallelLearningAgent la(le, set, params);
	la.init();

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_000.dot", la.getTPGGraph());

    auto start = std::chrono::system_clock::now();


    // Train for NB_GENERATIONS generations
	printf("Gen\tNbVert\tMin\tAvg\tMax\n");
	for (int i = 0; i < NB_GENERATIONS; i++) {
		char buff[12];
		sprintf(buff, "out_%03d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();
		std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> result;
		result = la.evaluateAllRoots(i, Learn::LearningMode::VALIDATION);
		auto iter = result.begin();
		double min = iter->first->getResult();
		std::advance(iter, result.size() - 1);
		double max = iter->first->getResult();
		double avg = std::accumulate(result.begin(), result.end(), 0.0,
			[](double acc, std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> pair)->double {return acc + pair.first->getResult(); });
		avg /= result.size();
		printf("%3d\t%4" PRIu64 "\t%1.2lf\t%1.2lf\t%1.2lf   -   ", i, la.getTPGGraph().getNbVertices(), min, avg, max);
        std::cout << "elapsed time : "
                  << ((std::chrono::duration<double>) (std::chrono::system_clock::now() - start)).count() << std::endl;
		la.trainOneGeneration(i);

	}

	// Keep best policy
	la.keepBestPolicy();
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();



	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

    // if we want to test the best agent
    if(true){
        test();
        return 0;
    }

	return 0;
}
