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
#include "descriptors.h"

void exportIndividual(const TPG::TPGVertex* vertex,
                      const std::string& basePathDots,
                      const std::string& basePathStats,
                      const size_t index,
                      uint64_t seed,
                      int indexParam,
                      const std::string& usecase,
                      std::shared_ptr<TPG::TPGGraph> graph,
                      File::TPGGraphDotExporter& dotExporter) {

    char dotFile[300];

    sprintf(dotFile, "%s/out_best_%zu.%" PRIu64 ".p%d.%s.dot",
            basePathDots.c_str(), index, seed, indexParam, usecase.c_str());
    dotExporter.setNewFilePath(dotFile);

    dotExporter.printSubGraph(vertex);

    TPG::PolicyStats ps;
    ps.setEnvironment(graph->getEnvironment());
    ps.analyzePolicy(vertex);

    char statsFile[300];
    sprintf(statsFile, "%s/out_best_stats_%zu.%" PRIu64 ".p%d.%s.md",
            basePathStats.c_str(), index, seed, indexParam, usecase.c_str());
    std::ofstream statsOut(statsFile);
    statsOut << ps;
    statsOut.close();
}

std::vector<size_t> parseAndFillArchiveValues(const std::string& archiveValuesStr) {
    std::vector<size_t> archiveValues;
    std::istringstream iss(archiveValuesStr);
    std::string segment;

    while (std::getline(iss, segment, '-')) {
        // Check integer
        bool isInteger = true;
        for (char c : segment) {
            if (!std::isdigit(c)) {
                isInteger = false;
                break;
            }
        }

        if (isInteger) {
            archiveValues.push_back(std::stoi(segment));
        } else {
            throw std::runtime_error("Berk type");
        }
    }

    return archiveValues;
}

void initializeArchiveParams(std::vector<std::string>& archiveDots,
                             std::vector<std::string>& archiveStats,
                             std::vector<std::shared_ptr<Selector::MapElites::MapElitesDescriptor>>& descriptors,
                             std::string descriptorTypeStr,
                             std::string logsFolder)
{

	if (descriptorTypeStr.empty()) {
		return;
	}

	std::istringstream iss(descriptorTypeStr);
    std::string token;

    while (std::getline(iss, token, ',')) {
        // Remove space if any
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());

        // Convert
		if(token == "ActionValues"){
        	descriptors.push_back(std::make_shared<Selector::MapElites::DefaultDescriptors::ActionValues>());
		} else if (token == "FeetContact"){
        	descriptors.push_back(std::make_shared<Selector::MapElites::CustomDescriptors::FeetContact>());
		} else {
			throw std::runtime_error("Descriptor type not found");
		}

        std::string dotPath  = logsFolder + "/archiveDots_"  + token;
        std::string statPath = logsFolder + "/archiveStats_" + token;

        archiveDots.push_back(dotPath);
        archiveStats.push_back(statPath);

		if (!std::filesystem::exists(dotPath)) {
			std::filesystem::create_directory(dotPath);
		}
		if (!std::filesystem::exists(statPath)) {
			std::filesystem::create_directory(statPath);
		}
    }

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
	bool useCVT = 0;

	std::string archiveValuesStr = "";
	std::string descriptorTypeStr = "";
	size_t sizeCVT = 1000;

	static struct option long_options[] = {
		{"cvt",      required_argument, 0,  6 },
		{"scvt",      required_argument, 0,  7 },
		{0, 0, 0, 0}
	};

    strcpy(logsFolder, "logs");
    strcpy(paramFile, "params_0.json");
	strcpy(usecase, "ant");
    strcpy(xmlFile, "none");
	int long_index = 0;
	while((option = getopt_long(argc, argv, "s:p:l:x:h:c:u:a:g:o:d:", long_options, &long_index)) != -1){
		switch (option) {
			case 's': seed= atoi(optarg); break;
			case 'p': strcpy(paramFile, optarg); break;
			case 'l': strcpy(logsFolder, optarg); break;
			case 'u': strcpy(usecase, optarg); break;
			case 'h': useHealthyReward = atoi(optarg); break;
			case 'x': strcpy(xmlFile, optarg); break;
			case 'a': archiveValuesStr = optarg; break;
			case 'g': saveAllGenerationsDots = atoi(optarg); break;
			case 'd': descriptorTypeStr = optarg; break;
			case 6: useCVT = atoi(optarg); break;                 // --cvt
			case 7: sizeCVT = atoi(optarg); break;                // --scvt
			default:
				std::cout << "Unrecognised option. Valid options are "
					"'-s seed' '-p paramFile.json' '-u useCase' "
					"'-l logsFolder' '-x xmlFile' '-h useHealthyReward' "
					"'-a sizeArchive' '-g saveAllGenDotFiles' "
					"'-d descriptorTypeStr' "
					"'--dMinMax useMinMaxDescriptor' '--cvt useCVT' '--scvt sizeCVT'."  << std::endl;
				exit(1);
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



    // Save the index of the parameter file.
    int indexParam = 0;
	std::string paramFileStr(paramFile);
	std::smatch match;
	std::regex re(R"((\d+)(?!.*\d))");

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


	// Export parameters before starting training.
	// These may differ from imported parameters because of LE or machine specific
	// settings such as thread count of number of actions.
	char jsonFilePath[200];  // Assurez-vous que ce soit assez grand pour contenir les deux parties concaténées.
	snprintf(jsonFilePath, sizeof(jsonFilePath), "%s/exported_params.%s.p%d.json", logsFolder, usecase, indexParam);
	File::ParametersParser::writeParametersToJson(jsonFilePath, params);
	


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

	


	// Instantiate and init the learning agent
	Learn::ParallelLearningAgent la(*mujocoLE, set, params);
	la.init(seed);


	
	std::vector<std::string> archiveDots;
	std::vector<std::string> archiveStats;
	

	// Creates descriptors
	std::vector<std::shared_ptr<Selector::MapElites::MapElitesDescriptor>> descriptors;
	initializeArchiveParams(archiveDots, archiveStats, descriptors, descriptorTypeStr, std::string(logsFolder));


	std::vector<size_t> allArchiveValues;
	if(!useCVT){
		allArchiveValues = parseAndFillArchiveValues(archiveValuesStr);
		if(descriptors.size() != allArchiveValues.size()){
			throw std::runtime_error("Difference between number of descriptors and number of archive values");
		}
	}

	std::map<std::shared_ptr<const Selector::MapElites::MapElitesDescriptor>, std::shared_ptr<Selector::MapElites::MapElitesArchive>> archives;
	if(params.selection._selectionMode == "mapElites") {
		std::shared_ptr<Selector::Selector> selector = la.getSelector();
		auto mapElitesSelector = std::dynamic_pointer_cast<Selector::MapElitesSelector>(selector);
		if(mapElitesSelector == nullptr){
			throw std::runtime_error("selector should be mapElites");
		}

		for(size_t idx = 0; idx < descriptors.size(); idx++) {
			descriptors[idx]->initDescriptor(*la.getTPGGraph(), *mujocoLE);
			if(useCVT){
				mapElitesSelector->addCvtArchiveFromDescriptor(sizeCVT, descriptors[idx], *mujocoLE, la.getRNG());
			} else {
				mapElitesSelector->addArchiveFromDescriptor(allArchiveValues[idx], descriptors[idx], *mujocoLE);
			}
		}
		archives = mapElitesSelector->getMapElitesArchives();
	}

	std::cout << "LOGGING FOLDER " << logsFolder << std::endl;
    std::cout << "SELECTED SEED : " << seed << std::endl;
    std::cout << "SELECTED PARAMS FILE: " << paramFile << std::endl;
	std::cout << "SELECTED DESCRIPTORS : ";

	
	std::cout << std::endl;
	std::cout << "SELECTION METHOD(S) :"<<std::endl;
	if(params.selection._selectionMode == "mapElites"){
		for(size_t idx = 0; idx < descriptors.size(); idx++){
			if(useCVT){
				
				std::cout<<" CVT MAP ELITES with " << archives.begin()->second->getDimensions().second << " dimensions and a size " << sizeCVT << std::endl;
			} else {
				std::cout<<" MAP ELITES";

				auto dims = archives.begin()->second->getDimensions();
				std::cout<<" with a "<<dims.first<<"-dimensional archive with "<<dims.second<<" dimensions resulting in size "<< (uint64_t)std::pow(dims.first, dims.second)<<std::endl;
			}
		}



	} else if(params.selection._selectionMode == "tournament"){
		std::cout<<" TOURNAMENT SELECTION"<<std::endl;
	} else {
		std::cout<<" TRUNCATION SELECTION"<<std::endl;
	}
	std::cout << "START MUJOCO APPLICATION WITH ENVIRONMENT "<< usecase << std::endl;
	std::cout << "NUMBER OF THREADS " << params.nbThreads << std::endl;


	// Basic logger
	Log::LABasicLogger basicLogger(la);

    // Basic Logger
    char logPath[250];
	sprintf(logPath, "%s/out.%" PRIu64 ".p%d.%s.std", logsFolder, seed, indexParam, usecase);


    std::ofstream logStream;
    logStream.open(logPath);
    Log::LABasicLogger log(la, logStream);


	std::vector<Log::MapElitesArchiveLogger*> loggerArchives;
	// Create the archive CSV file
	for(auto pair: archives){
		// Basic Logger
		char logArchive[250];
		sprintf(logArchive, "%s/archive_%s.%" PRIu64 ".p%d.%s.csv", logsFolder, pair.first->getName().c_str(), seed, indexParam, usecase);
		std::ofstream logArchiveStream;
		logArchiveStream.open(logArchive);
		Log::MapElitesArchiveLogger loggerArchive(*pair.second, la, logArchiveStream);
		loggerArchives.push_back(&loggerArchive);
	}


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
	for (uint64_t i = 0; i < params.nbGenerations; i++) {

		char buff[250];
		if(saveAllGenerationsDots){
			sprintf(buff, "%s/out_lastGen.%" PRIu64 ".%" PRIu64 ".p%d.%s.dot", dotGen, i, seed, indexParam, usecase);
		} else {
			sprintf(buff, "%s/out_lastGen.%" PRIu64 ".p%d.%s.dot", dotGen, seed, indexParam, usecase);
		}
		dotExporter.setNewFilePath(buff);
		dotExporter.print();

		la.trainOneGeneration(i, i + 1 != params.nbGenerations);	

		if(i % params.stepValidation == 0 && params.doValidation){

			size_t idxArchive = 0;
			for(auto& pair: archives){
				size_t indexElem = 0;
				for (auto elem: pair.second->getAllArchive()) {
					if (elem.second != nullptr && la.getTPGGraph()->hasVertex(*elem.second)) {
						exportIndividual(elem.second, archiveDots.at(idxArchive), archiveStats.at(idxArchive), indexElem, seed, indexParam, usecase,
										la.getTPGGraph(), dotExporter);
					}
					indexElem++;
				}
				idxArchive++;
			}
		}
	}
	
	la.getTPGGraph()->clearProgramIntrons();

	if (!archives.empty()) {
		size_t idxArchive = 0;
		for(auto& pair: archives){
			size_t indexElem = 0;
			for (auto elem: pair.second->getAllArchive()) {
				if (elem.second != nullptr && la.getTPGGraph()->hasVertex(*elem.second)) {
					exportIndividual(elem.second, archiveDots.at(idxArchive), archiveStats.at(idxArchive), indexElem, seed, indexParam, usecase,
									la.getTPGGraph(), dotExporter);
				}
				indexElem++;
			}
			idxArchive++;
		}

	} else {
		la.getSelector()->keepBestPolicy();
		const auto* best = la.getSelector()->getBestRoot().first;
		size_t index = 0;
		exportIndividual(best, logsFolder, logsFolder, index, seed, indexParam, usecase,
						la.getTPGGraph(), dotExporter);
	}


	// cleanup
	for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
		delete (&set.getInstruction(i));
	}

	return 0;
}