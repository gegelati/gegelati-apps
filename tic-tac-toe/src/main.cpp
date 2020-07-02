#include <unordered_set>
#include <string>
#include <cfloat>

#include <gegelati.h>

#include "TicTacToe.h"
#include "resultTester.h"

#ifndef NB_GENERATIONS
#define NB_GENERATIONS 200
#endif


int main() {
    // Create the instruction set for programs
    Instructions::Set set;
    auto minus = [](double a, double b) -> double { return (double) a - (double) b; };
    auto add = [](double a, double b) -> double { return a + b; };
    auto max = [](double a, double b) -> double { return std::max(a, b); };
    auto modulo = [](double a, double b) -> double { return b != 0.0 ? fmod(a, b) : DBL_MIN; };
    auto nulltest = [](double a) -> double { return (a == -1.0) ? 10.0 : 0.0; };
    auto circletest = [](double a) -> double { return (a == 0.0) ? 10.0 : 0.0; };
    auto crosstest = [](double a) -> double { return (a == 1.0) ? 10.0 : 0.0; };
    auto test15 = [](double a) -> double { return (a >= 15.0) ? 10.0 : 0.0; };
    auto cond = [](double a, double b) -> double { return a < b ? -a : a; };


    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo)));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));
    set.add(*(new Instructions::LambdaInstruction<double>(circletest)));
    set.add(*(new Instructions::LambdaInstruction<double>(crosstest)));
    set.add(*(new Instructions::LambdaInstruction<double>(test15)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(cond)));


    // Set the parameters for the learning process.
    // (Controls mutations probability, program lengths, and graph size
    // among other things)
    // Loads them from "params.json" file
    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json",params);

    // Instantiate the LearningEnvironment
    TicTacToe le;

    // Instantiate and init the learning agent
    Learn::ParallelLearningAgent la(le, set, params);
    la.init();

    // Adds a logger to the LA (to get statistics on learning) on std::cout
    auto logCout = *new Log::LABasicLogger();
    la.addLogger(logCout);

    // Adds another logger that will log in a file
    std::ofstream o("log");
    auto logFile = *new Log::LABasicLogger(o);
    la.addLogger(logFile);

    // Create an exporter for all graphs
    File::TPGGraphDotExporter dotExporter("out_000.dot", la.getTPGGraph());



    // Train for NB_GENERATIONS generations
    for (int i = 0; i < NB_GENERATIONS; i++) {
        char buff[12];
        sprintf(buff, "out_%03d.dot", i);
        dotExporter.setNewFilePath(buff);
        dotExporter.print();
        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> result;
        result = la.evaluateAllRoots(i, Learn::LearningMode::VALIDATION);

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
#ifndef NO_CONSOLE_CONTROL
        agentTest();
#endif        

    return 0;
}


