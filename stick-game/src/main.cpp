#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>
#include <cfloat>
#include <inttypes.h>

#include <gegelati.h>

#include "stickGameWithOpponent.h"

#ifndef NB_GENERATIONS
#define NB_GENERATIONS 300
#endif


int main() {
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
	File::ParametersParser::loadParametersFromJson("../../params.json",params);

	// Instantiate the LearningEnvironment
	StickGameWithOpponent le;

	// Instantiate and init the learning agent
	Learn::LearningAgent la(le, set, params);
	la.init();

	// Create an exporter for all graphs
	File::TPGGraphDotExporter dotExporter("out_000.dot", la.getTPGGraph());

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
		printf("%3d\t%4" PRIu64 "\t%1.2lf\t%1.2lf\t%1.2lf\n", i, la.getTPGGraph().getNbVertices(), min, avg, max);
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

	return 0;
}
