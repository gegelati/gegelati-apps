#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>

#include <gegelati.h>

#include "stickGameWithOpponent.h"


int main() {
	// Create the instruction set for programs
	Instructions::Set set;
	auto minus = [](int a, int b)->double {return (double)a - (double)b; };
	auto cast = [](int a, int b)->double {return (double)a; };
	auto add = [](double a, double b)->double {return a + b; };
	auto max = [](double a, double b)->double {return std::max(a, b); };
	auto nulltest = [](double a, double b)->double {return (a == 0.0) ? 10.0 : 0.0; };
	auto modulo = [](double a, double b)->double {
		if (b != 0.0) { return fmod(a, b); }
		else { return  DBL_MIN; }	};

	set.add(*(new Instructions::LambdaInstruction<double>(modulo)));
	set.add(*(new Instructions::LambdaInstruction<int>(minus)));
	set.add(*(new Instructions::LambdaInstruction<double>(add)));
	set.add(*(new Instructions::LambdaInstruction<int>(cast)));
	set.add(*(new Instructions::LambdaInstruction<double>(max)));
	set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	Learn::LearningParameters params;
	params.mutation.tpg.maxInitOutgoingEdges = 3;
	params.mutation.tpg.nbRoots = 50;
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
	params.archiveSize = 50;
	params.maxNbActionsPerEval = 11;
	params.nbIterationsPerPolicyEvaluation = 100;
	params.ratioDeletedRoots = 0.5;

	// Instantiate the LearningEnvironment
	StickGameWithOpponent le;

	// Instantiate and init the learning agent
	Learn::LearningAgent la(le, set, params);
	la.init();

	// Create an exporter for all graphs
	Exporter::TPGGraphDotExporter dotExporter("out_000.dot", la.getTPGGraph());

	// Train for 300 generations
	printf("Gen\tNbVert\tMin\tAvg\tMax\n");
	for (int i = 0; i < 300; i++) {
		char buff[11];
		sprintf(buff, "out_%3d.dot", i);
		dotExporter.setNewFilePath(buff);
		dotExporter.print();
		std::multimap<double, const TPG::TPGVertex*> result;
		result = la.evaluateAllRoots(i);
		auto iter = result.begin();
		double min = iter->first;
		std::advance(iter, result.size() - 1);
		double max = iter->first;
		double avg = std::accumulate(result.begin(), result.end(), 0.0,
			[](double acc, std::pair<double, const TPG::TPGVertex*> pair)->double {return acc + pair.first; });
		avg /= result.size();
		printf("%3d\t%4lld\t%1.2lf\t%1.2lf\t%1.2lf\n", i, la.getTPGGraph().getNbVertices(), min, avg, max);
		la.trainOneGeneration(i);

	}

	// Keep best policy
	la.keepBestPolicy();
	dotExporter.setNewFilePath("out_best.dot");
	dotExporter.print();

	// cleanup
	for (int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}