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
#include "lexicaseSelector.h"

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
    dotExporter.print();

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
	bool useObstacleReward = 0;
	bool saveAllGenerationsDots = 1;
	std::string obstacleUsedStr = "";
	char dotPath[150];

    strcpy(dotPath, "");
    strcpy(logsFolder, "logs");
    strcpy(paramFile, "params_0.json");
	strcpy(usecase, "ant");
    strcpy(xmlFile, "none");
    while((option = getopt(argc, argv, "s:p:l:x:h:c:u:a:g:w:o:d:")) != -1){
        switch (option) {
            case 's': seed= atoi(optarg); break;
            case 'p': strcpy(paramFile, optarg); break;
            case 'l': strcpy(logsFolder, optarg); break;
			case 'u': strcpy(usecase, optarg); break;
			case 'h': useHealthyReward = atoi(optarg); break;
            case 'x': strcpy(xmlFile, optarg); break;
			case 'g': saveAllGenerationsDots = atoi(optarg); break;
			case 'o': useObstacleReward = atoi(optarg); break;
			case 'w': obstacleUsedStr = optarg; break;
            case 'd': strcpy(dotPath, optarg); break;
            default: std::cout << "Unrecognised option. Valid options are \'-s seed\' \'-p paramFile.json\' \'-u useCase\' \'-logs logs Folder\'  \'-x xmlFile\' \'-h useHealthyReward\' \'-g saveAllGenDotFiles\' \'-w obstacle used \'." << std::endl; exit(1);
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



    std::vector<std::vector<size_t>> obstacleUsed;
    std::stringstream ss(obstacleUsedStr);
    std::string token;

    while (std::getline(ss, token, ',')) {
        if (token.empty()) continue;

        std::vector<size_t> currentGroup;
        for (char c : token) {
            if (isdigit(c)) {
                currentGroup.push_back(c - '0');
            } else {
                std::cerr << "Caractère invalide dans le token: " << c << std::endl;
                exit(1);
            }
        }
        obstacleUsed.push_back(currentGroup);
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
	if(params.selection._selectionMode == "tournament"){
		std::cout<<" TOURNAMENT SELECTION"<<std::endl;
	} else if(params.selection._selectionMode == "truncation") {
		std::cout<<" TRUNCATION SELECTION"<<std::endl;
	} else if(params.selection._selectionMode == "lexicase"){
		std::cout<<" LEXICASE SELECTION"<<std::endl;
	} else {
		std::cout<<" NO SELECTION.... WAIIIT I SHOULD CRASH NOOW?"<<std::endl;
	}


	std::cout << "START MUJOCO APPLICATION WITH ENVIRONMENT "<< usecase << std::endl;

	if((obstacleUsed.size() == 0)){
		std::cout << "USING NO OBSTACLE " << std::endl;
		obstacleUsed.clear();
	} else {
		std::cout << "USING OBSTACLES: ";
		for (const auto& group : obstacleUsed) {
			for (size_t num : group) {
				std::cout << num << " ";
			}
			std::cout << "| ";
		}
		std::cout<<std::endl;
	}

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
		mujocoLE = new MujocoHalfCheetahWrapper(xmlFile, useObstacleReward);
	} else if (strcmp(usecase, "hopper") == 0) {
		mujocoLE = new MujocoHopperWrapper(xmlFile, useHealthyReward);
	} else if (strcmp(usecase, "walker2d") == 0) {
		mujocoLE = new MujocoWalker2DWrapper(xmlFile, useHealthyReward, useObstacleReward);
	} else if (strcmp(usecase, "inverted_double_pendulum") == 0) {
		mujocoLE = new MujocoDoublePendulumWrapper(xmlFile);
	} else if (strcmp(usecase, "reacher") == 0) {
		mujocoLE = new MujocoReacherWrapper(xmlFile);
	} else if (strcmp(usecase, "ant") == 0) {
		mujocoLE = new MujocoAntWrapper(xmlFile, useHealthyReward);
	} else {
		throw std::runtime_error("Use case not found");
	}


	std::cout << "NUMBER OF THREADS: " << params.nbThreads << std::endl;

	// Name of csv file for validation data with logsFolder to create it, and usecase and indexparam
	std::string csvFile = "" + std::string(logsFolder) + "/validationStats." + std::to_string(seed) + "." + std::string(usecase) + ".p" + std::to_string(indexParam) + ".csv";

	// Instantiate and init the learning agent
	Learn::LexicaseAgent la(*mujocoLE, set, params, obstacleUsed, csvFile);
	la.init(seed);

	if(dynamic_cast<Selector::LexicaseSelector*>(la.getSelector().get()) == nullptr && params.selection._selectionMode == "lexicase"){
		throw std::runtime_error("lexicase selector should be used.");
	}

	
    std::string dotFileName = std::filesystem::path(dotPath).stem().string();
	if(dotFileName.size() > 0){
		std::cout<<"LOAD FILE " << dotPath << std::endl;
		auto &tpg = *la.getTPGGraph();
		Environment env(set, params, mujocoLE->getDataSources(), mujocoLE->getNbActions());

		File::TPGGraphDotImporter dotImporter(dotPath, env, tpg);
	}

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
    char dotSavingPath[400];
	if(saveAllGenerationsDots){
    	sprintf(dotSavingPath, "%s/out_lastGen.%" PRIu64 ".0.p%d.%s.dot", dotGen, seed, indexParam, usecase);
	} else {
    	sprintf(dotSavingPath, "%s/out_lastGen.%" PRIu64 ".p%d.%s.dot", dotGen, seed, indexParam, usecase);	
	}
	File::TPGGraphDotExporter dotExporter(dotSavingPath, *la.getTPGGraph());

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

		la.trainOneGeneration(i, i != params.nbGenerations - 1);
	}

	la.getTPGGraph()->clearProgramIntrons();

	la.getSelector()->keepBestPolicy();
	const auto* best = la.getSelector()->getBestRoot().first;
	std::vector<size_t> emptyIndices;
	exportIndividual(best, logsFolder, logsFolder, emptyIndices, seed, indexParam, usecase,
					la.getTPGGraph(), dotExporter);


    bool printCodeGen = true;
    if(printCodeGen){
		char codeGen[160];
		snprintf(codeGen, sizeof(codeGen), "%s/codeGen/", logsFolder);
    
        if(!std::filesystem::exists(codeGen)){
            std::filesystem::create_directory(codeGen);
        }

        std::cout << "Printing C code." << std::endl;
        CodeGen::TPGGenerationEngineFactory factory(CodeGen::TPGGenerationEngineFactory::switchMode);
        std::unique_ptr<CodeGen::TPGGenerationEngine> tpggen = factory.create("codeGen", *la.getTPGGraph(), codeGen);
        tpggen->generateTPGGraph();
    }

	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}