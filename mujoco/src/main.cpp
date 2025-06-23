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

void exportIndividual(const TPG::TPGVertex* vertex,
                      const std::string& basePathDots,
                      const std::string& basePathStats,
                      const std::vector<size_t>& indices,
                      uint64_t seed,
                      int indexParam,
                      const std::string& usecase,
                      std::shared_ptr<TPG::TPGGraph> graph,
                      File::TPGGraphDotExporter& dotExporter) {

    char dotFile[300];
    std::ostringstream suffix;
    for (const auto& idx : indices) {
        suffix << "_" << idx;
    }

    sprintf(dotFile, "%s/out_best%s.%" PRIu64 ".p%d.%s.dot",
            basePathDots.c_str(), suffix.str().c_str(), seed, indexParam, usecase.c_str());
    dotExporter.setNewFilePath(dotFile);
    dotExporter.printSubGraph(vertex);

    TPG::PolicyStats ps;
    ps.setEnvironment(graph->getEnvironment());
    ps.analyzePolicy(vertex);

    char statsFile[300];
    sprintf(statsFile, "%s/out_best_stats%s.%" PRIu64 ".p%d.%s.md",
            basePathStats.c_str(), suffix.str().c_str(), seed, indexParam, usecase.c_str());
    std::ofstream statsOut(statsFile);
    statsOut << ps;
    statsOut.close();
}



int main(int argc, char ** argv) {

    char option;
    uint64_t seed = 0;
    char paramFile[1500];
	char logsFolder[150];
	char xmlFile[150];
	char usecase[150];
	bool useHealthyReward = 1;
	bool saveAllGenerationsDots = 1;

    strcpy(logsFolder, "logs");
    strcpy(paramFile, "params_0.json");
	strcpy(usecase, "ant");
    strcpy(xmlFile, "none");
    while((option = getopt(argc, argv, "s:p:l:x:h:c:u:a:g:w:o:")) != -1){
        switch (option) {
            case 's': seed= atoi(optarg); break;
            case 'p': strcpy(paramFile, optarg); break;
            case 'l': strcpy(logsFolder, optarg); break;
			case 'u': strcpy(usecase, optarg); break;
			case 'h': useHealthyReward = atoi(optarg); break;
            case 'x': strcpy(xmlFile, optarg); break;
			case 'g': saveAllGenerationsDots = atoi(optarg); break;
            default: std::cout << "Unrecognised option. Valid options are \'-s seed\' \'-p paramFile.json\' \'-u useCase\' \'-logs logs Folder\'  \'-x xmlFile\' \'-h useHealthyReward\' \'-g saveAllGenDotFiles\'." << std::endl; exit(1);
        }
    }
	if(strcmp(xmlFile, "none") == 0){
    	snprintf(xmlFile, sizeof(xmlFile), "mujoco_models/%s.xml", usecase);
	}

	char dotGen[160];
	snprintf(dotGen, sizeof(dotGen), "%s/dotFiles", logsFolder);

	// Create log folder if needed
	if (!std::filesystem::exists(logsFolder)) {
		std::filesystem::create_directory(logsFolder);
	}
	// Create dot per generation folder if needed
	if (!std::filesystem::exists(dotGen)) {
		std::filesystem::create_directory(dotGen);
	}

    std::cout << "SELECTED SEED : " << seed << std::endl;
    std::cout << "SELECTED PARAMS FILE: " << paramFile << std::endl;


    // Save the index of the parameter file.
    int indexParam = 0;
	std::string paramFileStr(paramFile);
	std::smatch match;
	std::regex re(R"((\d+)(?!.*\d))"); // Capture le dernier groupe de chiffres

	if (std::regex_search(paramFileStr, match, re)) {
		indexParam = std::stoi(match[1]);
	} else {
		throw std::runtime_error("error detection of index");
	}



	// Create the instruction set for programs
	Instructions::Set set;
	fillInstructionSet(set);

	// Set the parameters for the learning process.
	// (Controls mutations probability, program lengths, and graph size
	// among other things)
	// Loads them from the file params.json
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(paramFile, params);

	std::cout << "SELECTION METHOD :";
	if(params.useTournamentSelection){
		std::cout<<" TOURNAMENT SELECTION"<<std::endl;
	} else {
		std::cout<<" STANDARD SELECTION"<<std::endl;
	}


	std::cout << "START MUJOCO APPLICATION WITH ENVIRONMENT "<< usecase << std::endl;

	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	char jsonFilePath[200];  // Assurez-vous que ce soit assez grand pour contenir les deux parties concaténées.
	snprintf(jsonFilePath, sizeof(jsonFilePath), "%s/exported_params.%s.p%d.json", logsFolder, usecase, indexParam);

	// Instantiate the LearningEnvironment
	MujocoWrapper* mujocoLE = nullptr;
	if(strcmp(usecase, "humanoid") == 0){
		mujocoLE = new MujocoHumanoidWrapper(xmlFile, useHealthyReward);
	} else if (strcmp(usecase, "half_cheetah") == 0) {
		mujocoLE = new MujocoHalfCheetahWrapper(xmlFile);
	} else if (strcmp(usecase, "hopper") == 0) {
		mujocoLE = new MujocoHopperWrapper(xmlFile, useHealthyReward);
	} else if (strcmp(usecase, "walker2d") == 0) {
		mujocoLE = new MujocoWalker2DWrapper(xmlFile, useHealthyReward);
	} else if (strcmp(usecase, "inverted_double_pendulum") == 0) {
		mujocoLE = new MujocoDoublePendulumWrapper(xmlFile);
	} else if (strcmp(usecase, "reacher") == 0) {
		mujocoLE = new MujocoReacherWrapper(xmlFile);
	} else if (strcmp(usecase, "ant") == 0) {
		mujocoLE = new MujocoAntWrapper(xmlFile, useHealthyReward);
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
	if(saveAllGenerationsDots){
    	sprintf(dotPath, "%s/out_lastGen.%" PRIu64 ".0.p%d.%s.dot", dotGen, seed, indexParam, usecase);
	} else {
    	sprintf(dotPath, "%s/out_lastGen.%" PRIu64 ".p%d.%s.dot", dotGen, seed, indexParam, usecase);	
	}
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

		char buff[250];
		if(saveAllGenerationsDots){
			sprintf(buff, "%s/out_lastGen.%" PRIu64 ".%" PRIu64 ".p%d.%s.dot", dotGen, i, seed, indexParam, usecase);
		} else {
			sprintf(buff, "%s/out_lastGen.%" PRIu64 ".p%d.%s.dot", dotGen, seed, indexParam, usecase);
		}
		dotExporter.setNewFilePath(buff);
		dotExporter.print();

		la.trainOneGeneration(i);
	}

	
	la.getTPGGraph()->clearProgramIntrons();


	la.keepBestPolicy();
	const auto* best = la.getBestRoot().first;
	std::vector<size_t> emptyIndices;
	exportIndividual(best, logsFolder, logsFolder, emptyIndices, seed, indexParam, usecase,
					la.getTPGGraph(), dotExporter);



	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}