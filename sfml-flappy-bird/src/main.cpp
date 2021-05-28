#include <iostream>
#include <thread>
#include <atomic>
//#include <chrono>
#include <cinttypes>
#include "Flappy_bird.h"
#include "Render.h"


int main() {

    std::cout << "Start flappy Bird application." << std::endl;

    // Create the instruction set for programs
    Instructions::Set set;
    auto minus = [](double a, double b)->double {return a - b; };
    auto add = [](double a, double b)->double {return a + b; };
    auto mult = [](double a, double b)->double {return a * b; };
    auto div = [](double a, double b)->double {return a / b; };
    auto max = [](double a, double b)->double {return std::max(a, b); };
    auto ln = [](double a)->double {return std::log(a); };
    auto exp = [](double a)->double {return std::exp(a); };
    auto cos = [](double a)->double {return std::cos(a); };
    auto sin = [](double a)->double {return std::sin(a); };
    auto tan = [](double a)->double {return std::tan(a); };
    auto pi = [](double a)->double {return M_PI; };
//    auto multByConst = [](double a, Data::Constant c)->double {return a * (double)c / 10.0; };

    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double>(exp)));
    set.add(*(new Instructions::LambdaInstruction<double>(ln)));
    set.add(*(new Instructions::LambdaInstruction<double>(cos)));
    set.add(*(new Instructions::LambdaInstruction<double>(sin)));
    set.add(*(new Instructions::LambdaInstruction<double>(tan)));
//    set.add(*(new Instructions::LambdaInstruction<double, Data::Constant>(multByConst)));
    set.add(*(new Instructions::LambdaInstruction<double>(pi)));

    // Set the parameters for the learning process.
    // (Controls mutations probability, program lengths, and graph size
    // among other things)
    // Loads them from the file params.json
    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
#ifdef NB_GENERATIONS
    params.nbGenerations = NB_GENERATIONS;
#endif

    // Instantiate the LearningEnvironment
    Flappy_bird flappyBirdLE = Flappy_bird();

    std::cout << "Number of threads: " << params.nbThreads << std::endl;

    // Instantiate and init the learning agent
    Learn::ParallelLearningAgent la(flappyBirdLE, set, params);
//    Learn::LearningAgent la(flappyBirdLE, set, params);
    la.init();

    const TPG::TPGVertex* bestRoot = nullptr;

    // Start a thread for controlling the loop
    std::atomic<bool> doDisplay = false;
#ifndef NO_CONSOLE_CONTROL
    // Console
    std::atomic<bool> exitProgram = true; // (set to false by other thread)
    std::atomic<bool> toggleDisplay = true;
    std::atomic<uint64_t> generation = 0;

    std::thread threadDisplay(Render::controllerLoop, std::ref(exitProgram), std::ref(toggleDisplay), std::ref(doDisplay),
                              &bestRoot, std::ref(set), std::ref(flappyBirdLE), std::ref(params), std::ref(generation));

    while (exitProgram); // Wait for other thread to print key info.
#else
    std::atomic<bool> exitProgram = false; // (set to false by other thread)
	std::atomic<bool> toggleDisplay = false;
#endif
    // Basic logger1
    Log::LABasicLogger basicLogger(la);

    // Create an exporter for all graphs
    File::TPGGraphDotExporter dotExporter("out_0000.dot", la.getTPGGraph());

    // Logging best policy stat.
    std::ofstream stats;
    stats.open("bestPolicyStats.md");
    Log::LAPolicyStatsLogger policyStatsLogger(la, stats);

    // Train for params.nbGenerations generations
    for (int i = 0; i < params.nbGenerations; i++) {
        char buff[13];
        sprintf(buff, "out_%04d.dot", i);
        dotExporter.setNewFilePath(buff);
        dotExporter.print();
        while (doDisplay){} // waiting for the end of the computation of the replay
        la.trainOneGeneration(i);

#ifndef NO_CONSOLE_CONTROL
        generation = i;
        if (toggleDisplay && !exitProgram) {
            bestRoot = la.getBestRoot().first;
            doDisplay = true;
//            Render::controllerLoop(exitProgram, toggleDisplay, doDisplay,
//            &bestRoot, set, flappyBirdLE, params, generation);

            while (doDisplay && !exitProgram);
        }
#endif
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

    // cleanup
    for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
        delete (&set.getInstruction(i));
    }

#ifndef NO_CONSOLE_CONTROL
    // Exit the thread
    std::cout << "Exiting program, press a key then [enter] to exit if nothing happens.";
    threadDisplay.join();
#endif

    return 0;
}