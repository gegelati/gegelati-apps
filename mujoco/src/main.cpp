#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <inttypes.h>
#include <getopt.h>
#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>
#include <filesystem>

#include "mujocoEnvironment/mujocoWrappers.h"
#include "instructions.h"

int main(int argc, char ** argv) {

    char option;
    uint64_t seed = 0;
    char paramFile[1500];
	char logsFolder[150];
	char xmlFile[150];
	char usecase[150];
	bool useHealthyReward = 1;
	bool useContactForce = 0;
    strcpy(logsFolder, "logs");
    strcpy(paramFile, "params_0.json");
	strcpy(usecase, "ant");
    strcpy(xmlFile, "none");
    while((option = getopt(argc, argv, "s:p:l:x:h:c:u:")) != -1){
        switch (option) {
            case 's': seed= atoi(optarg); break;
            case 'p': strcpy(paramFile, optarg); break;
            case 'l': strcpy(logsFolder, optarg); break;
			case 'u': strcpy(usecase, optarg); break;
			case 'h': useHealthyReward = atoi(optarg); break;
			case 'c': useContactForce = atoi(optarg); break;
            case 'x': strcpy(xmlFile, optarg); break;
            default: std::cout << "Unrecognised option. Valid options are \'-s seed\' \'-p paramFile.json\' \'-u useCase\' \'-logs logs Folder\'  \'-x xmlFile\' \'-h useHealthyReward\' \'-c useContactForce\'." << std::endl; exit(1);
        }
    }
	if(strcmp(xmlFile, "none") == 0){
    	snprintf(xmlFile, sizeof(xmlFile), "mujoco_models/%s.xml", usecase);
	}

	// Create log folder if needed
	if (!std::filesystem::exists(logsFolder)) {
		std::filesystem::create_directory(logsFolder);
	}


    std::cout << "Selected seed : " << seed << std::endl;
    std::cout << "Selected params: " << paramFile << std::endl;

    // Save the index of the parameter file.
    int indexParam = std::stoi(std::regex_replace(paramFile, std::regex(R"(.*params_(\d+)\.json)"), "$1"));

	std::cout << "Start Mujoco application." << std::endl;

	// Create the instruction set for programs
	Instructions::Set set;
	fillInstructionSet(set);

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(paramFile, params);


	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	char jsonFilePath[200];  // Assurez-vous que ce soit assez grand pour contenir les deux parties concaténées.
	snprintf(jsonFilePath, sizeof(jsonFilePath), "%s/exported_params.p%d.json", logsFolder, indexParam);

	// Instantiate the LearningEnvironment
	MujocoWrapper* mujocoLE = nullptr;
	if(strcmp(usecase, "humanoid") == 0){
		mujocoLE = new MujocoHumanoidWrapper(xmlFile, useHealthyReward, useContactForce);
	} else if (strcmp(usecase, "half_cheetah") == 0) {
		mujocoLE = new MujocoHalfCheetahWrapper(xmlFile);
	} else if (strcmp(usecase, "hopper") == 0) {
		mujocoLE = new MujocoHopperWrapper(xmlFile);
	} else if (strcmp(usecase, "walker2d") == 0) {
		mujocoLE = new MujocoWalker2DWrapper(xmlFile);
	} else if (strcmp(usecase, "inverted_double_pendulum") == 0) {
		mujocoLE = new MujocoDoublePendulumWrapper(xmlFile);
	} else if (strcmp(usecase, "reacher") == 0) {
		mujocoLE = new MujocoReacherWrapper(xmlFile);
	} else if (strcmp(usecase, "ant") == 0) {
		mujocoLE = new MujocoAntWrapper(xmlFile, useHealthyReward, useContactForce);
	} else {
		throw std::runtime_error("Use case not found");
	}

	std::cout << "Number of threads: " << params.nbThreads << std::endl;

	// Instantiate and init the learning agent
	Learn::ParallelLearningAgent la(*mujocoLE, set, params);
	la.init(seed);


	std::atomic<bool> exitProgram = false; // (set to false by other thread) 
	std::atomic<bool> toggleDisplay = false;

	// Basic logger
	Log::LABasicLogger basicLogger(la);

    // Basic Logger
    char logPath[250];
	sprintf(logPath, "%s/out.%" PRIu64 ".p%d.%s.std", logsFolder, seed, indexParam, usecase);


    std::ofstream logStream;
    logStream.open(logPath);
    Log::LABasicLogger log(la, logStream);

	// Create an exporter for all graphs
    char dotPath[400];
    sprintf(dotPath, "%s/out_lastGen.%" PRIu64 ".p%d.%s.dot", logsFolder, seed, indexParam, usecase);
	File::TPGGraphDotExporter dotExporter(dotPath, *la.getTPGGraph());

	// Logging best policy stat.
    char bestPolicyStatsPath[250];
    sprintf(bestPolicyStatsPath, "%s/bestPolicyStats.%" PRIu64 ".p%d.%s.md", logsFolder, seed, indexParam, usecase);
	std::ofstream stats;
	stats.open(bestPolicyStatsPath);
	Log::LAPolicyStatsLogger policyStatsLogger(la, stats);


	File::ParametersParser::writeParametersToJson(jsonFilePath, params);

	// Train for params.nbGenerations generations
	for (uint64_t i = 0; i < params.nbGenerations && !exitProgram; i++) {
#define PRINT_ALL_DOT 1
#if PRINT_ALL_DOT
		if(i % 1 == 0){
			char buff[250];
			sprintf(buff, "%s/out_lastGen.%" PRIu64 ".%" PRIu64 ".p%d.%s.dot", logsFolder, i, seed, indexParam, usecase);
			dotExporter.setNewFilePath(buff);
			dotExporter.print();
		}

#endif


		la.trainOneGeneration(i);
	}

	
	// Keep best policy and clear graph
	la.keepBestPolicy();
	//la.getTPGGraph()->clearProgramIntrons();

    char bestDot[250];
	// Export the graph
    sprintf(bestDot, "%s/out_best.%" PRIu64 ".p%d.%s.dot", logsFolder, seed, indexParam, usecase);
	dotExporter.setNewFilePath(bestDot);
	dotExporter.print();

	TPG::PolicyStats ps;
	ps.setEnvironment(la.getTPGGraph()->getEnvironment());
	ps.analyzePolicy(la.getBestRoot().first);
	std::ofstream bestStats;
    sprintf(bestPolicyStatsPath, "%s/out_best_stats.%" PRIu64 ".p%d.%s.md", logsFolder, seed, indexParam, usecase);
	bestStats.open(bestPolicyStatsPath);
	bestStats << ps;
	bestStats.close();
	stats.close();

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}