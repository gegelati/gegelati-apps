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
#include "mujocoMapEliteAgent.h"
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
	if(vertex == nullptr){
		std::cout<<"meh"<<std::endl;
	}
	if(!graph->hasVertex(*vertex)){
		std::cout<<"meeeej"<<std::endl;
	}
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

std::vector<std::vector<double>> parseAndFillArchiveValues(const std::string& archiveValuesStr) {
    std::vector<std::vector<double>> archiveValues;
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
            // Integer case
            int n = std::stoi(segment);
            std::vector<double> subVector;
            for (int i = 1; i <= n; ++i) {
                double value = (n == 0) ? 0.0 : static_cast<double>(i) / n;
                subVector.push_back(value);
            }
            archiveValues.push_back(subVector);
        } else {
            // Value case
            std::vector<double> subVector;
            std::istringstream segStream(segment);
            std::string valueStr;
            while (std::getline(segStream, valueStr, ',')) {
                subVector.push_back(std::stod(valueStr));
            }
            archiveValues.push_back(subVector);
        }
    }

    return archiveValues;
}

void initializeArchiveParams(std::vector<std::string>& archiveDots,
                             std::vector<std::string>& archiveStats,
                             std::vector<Descriptor::DescriptorType>& descriptorTypes,
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
        descriptorTypes.push_back(Descriptor::StringToDescriptorType(token));
    }

    // Build archive file paths for each descriptor type
    for (auto descriptorType : descriptorTypes) {
        std::string dotPath  = logsFolder + "/archiveDots_"  + Descriptor::descriptorTypeToString(descriptorType);
        std::string statPath = logsFolder + "/archiveStats_" + Descriptor::descriptorTypeToString(descriptorType);

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
	bool useOnlyCloseAddEdges = 0;
	bool useCVT = 0;
	bool useMeanDescriptor = 0;
	bool useMedianDescriptor = 0;
	bool useAbsMeanDescriptor = 1;
	bool useQuantileDescriptor = 0;
	bool useMinMaxDescriptor = 0;
	
	bool useMainMeanDescriptor = 0;
	bool useMainMedianDescriptor = 0;
	bool useMainStdDescriptor = 0;
	bool useMainMaxDescriptor = 0;
	bool useMainMinDescriptor = 0;

	std::string archiveValuesStr = "";
	std::string descriptorTypeStr = "";
	size_t sizeCVT = 1000;

	static struct option long_options[] = {
		{"dMean",    required_argument, 0,  1 },
		{"dMed",     required_argument, 0,  2 },
		{"dAbsMean", required_argument, 0,  3 },
		{"dQ",       required_argument, 0,  4 },
		{"dMinMax",  required_argument, 0,  5 },
		{"cvt",      required_argument, 0,  6 },
		{"scvt",      required_argument, 0,  7 },
		{"dMainMean", required_argument, 0,  8 },
		{"dMainMed", required_argument, 0,  9 },
		{"dMainStd", required_argument, 0, 10 },
		{"dMainMax", required_argument, 0, 11 },
		{"dMainMin", required_argument, 0, 12 },
		{"dTypeProg", required_argument, 0, 13 },
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
			case 'o': useOnlyCloseAddEdges = atoi(optarg); break;
			case 'd': descriptorTypeStr = optarg; break;
			case 1: useMeanDescriptor = atoi(optarg); break;      // --dMean
			case 2: useMedianDescriptor = atoi(optarg); break;    // --dMed
			case 3: useAbsMeanDescriptor = atoi(optarg); break;   // --dAbsMean
			case 4: useQuantileDescriptor = atoi(optarg); break;  // --dQ
			case 5: useMinMaxDescriptor = atoi(optarg); break;    // --dMinMax
			case 6: useCVT = atoi(optarg); break;                 // --cvt
			case 7: sizeCVT = atoi(optarg); break;                // --scvt
			case 8: useMainMeanDescriptor = atoi(optarg); break;  // --dMainMean
			case 9: useMainMedianDescriptor = atoi(optarg); break; // --dMainMed
			case 10: useMainStdDescriptor = atoi(optarg); break;   // --dMainStd
			case 11: useMainMaxDescriptor = atoi(optarg); break;   // --dMainMax
			case 12: useMainMinDescriptor = atoi(optarg); break;   // --dMainMin
			default:
				std::cout << "Unrecognised option. Valid options are "
					"'-s seed' '-p paramFile.json' '-u useCase' "
					"'-l logsFolder' '-x xmlFile' '-h useHealthyReward' "
					"'-a sizeArchive' '-g saveAllGenDotFiles' "
					"'-o useOnlyCloseAddEdges' '-d descriptorTypeStr' "
					"'--dMean useMeanDescriptor' '--dMed useMedianDescriptor' "
					"'--dAbsMean useAbsMeanDescriptor' '--dQ useQuantileDescriptor' "
					"'--dMinMax useMinMaxDescriptor' '--cvt useCVT' '--scvt sizeCVT' "
					"'--dMainMean useMainMeanDescriptor' '--dMainMed useMainMedianDescriptor' "
					"'--dMainStd useMainStdDescriptor' '--dMainMax useMainMaxDescriptor' "
					"'--dMainMin useMainMinDescriptor'."  << std::endl;
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


	
	std::vector<std::string> archiveDots;
	std::vector<std::string> archiveStats;
	

	std::vector<Descriptor::DescriptorType> descriptorTypes;
	initializeArchiveParams(archiveDots, archiveStats, descriptorTypes, descriptorTypeStr, std::string(logsFolder));
	std::vector<std::vector<double>> allArchiveValues;
	if(!useCVT){
		allArchiveValues = parseAndFillArchiveValues(archiveValuesStr);
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


	// Instantiate the LearningEnvironment
	MujocoWrapper* mujocoLE = nullptr;
	if(strcmp(usecase, "humanoid") == 0){
		mujocoLE = new MujocoHumanoidWrapper(xmlFile, descriptorTypes, useHealthyReward);
	} else if (strcmp(usecase, "half_cheetah") == 0) {
		mujocoLE = new MujocoHalfCheetahWrapper(xmlFile, descriptorTypes);
	} else if (strcmp(usecase, "hopper") == 0) {
		mujocoLE = new MujocoHopperWrapper(xmlFile, descriptorTypes, useHealthyReward);
	} else if (strcmp(usecase, "walker2d") == 0) {
		mujocoLE = new MujocoWalker2DWrapper(xmlFile, descriptorTypes, useHealthyReward);
	} else if (strcmp(usecase, "inverted_double_pendulum") == 0) {
		mujocoLE = new MujocoDoublePendulumWrapper(xmlFile, descriptorTypes);
	} else if (strcmp(usecase, "reacher") == 0) {
		mujocoLE = new MujocoReacherWrapper(xmlFile, descriptorTypes);
	} else if (strcmp(usecase, "ant") == 0) {
		mujocoLE = new MujocoAntWrapper(xmlFile, descriptorTypes, useHealthyReward);
	} else {
		throw std::runtime_error("Use case not found");
	}

	if(!useCVT && descriptorTypes.size() != allArchiveValues.size()){
		throw std::runtime_error("Difference between number of descriptors and number of archive values");
	}

	std::map<Descriptor::DescriptorType ,const ArchiveParametrization*> archiveParams;
	for(size_t idx = 0; idx < descriptorTypes.size(); idx++){
		Descriptor::DescriptorType& descriptorType = descriptorTypes.at(idx);
		std::vector<double> archiveValues;
		if(useCVT){
			archiveParams.insert(std::make_pair(descriptorType, new CVTArchiveParametrization(
			mujocoLE->getNbDescriptors().at(descriptorType), archiveValues, 0.0, 1.0, sizeCVT, descriptorType,
			
			// Descriptor statistics options
			useMeanDescriptor, useMedianDescriptor,
			useAbsMeanDescriptor, useQuantileDescriptor, useMinMaxDescriptor, 

			// Main descriptor statistics options
			useMainMeanDescriptor, useMainMedianDescriptor,
			useMainStdDescriptor, useMainMaxDescriptor, useMainMinDescriptor
			)));
		} else {
			archiveValues = allArchiveValues.at(idx);
			archiveParams.insert(std::make_pair(descriptorType, new ArchiveParametrization(
			mujocoLE->getNbDescriptors().at(descriptorType), archiveValues, descriptorType,
			
			// Descriptor statistics options
			useMeanDescriptor, useMedianDescriptor,
			useAbsMeanDescriptor, useQuantileDescriptor, useMinMaxDescriptor, 

			// Main descriptor statistics options
			useMainMeanDescriptor, useMainMedianDescriptor,
			useMainStdDescriptor, useMainMaxDescriptor, useMainMinDescriptor
			)));
		}
	}
	


	// Instantiate and init the learning agent
	Learn::MujocoMapEliteLearningAgent la(*mujocoLE, set, 
		archiveParams, params, useOnlyCloseAddEdges);
	la.init(seed);


	std::cout << "LOGGING FOLDER " << logsFolder << std::endl;
    std::cout << "SELECTED SEED : " << seed << std::endl;
    std::cout << "SELECTED PARAMS FILE: " << paramFile << std::endl;
	std::cout << "SELECTED DESCRIPTORS : ";
	for(auto descriptorType: descriptorTypes){
		std::cout<<Descriptor::descriptorTypeToString(descriptorType)<<" - ";
	}

	
	std::cout << std::endl;
	std::cout << "SELECTION METHOD(S) :"<<std::endl;
	if(descriptorTypes.size() > 0){
		for(size_t idx = 0; idx < descriptorTypes.size(); idx++){
			if(useCVT){
				std::cout<<" CVT MAP ELITES with " << la.getMapElitesArchiveAt(descriptorTypes.at(idx)).getDimensions().second << " dimensions and a size " << la.getMapElitesArchiveAt(descriptorTypes.at(idx)).size() << std::endl;
			} else {
				std::cout<<" MAP ELITES";

				auto dims = la.getMapElitesArchiveAt(descriptorTypes.at(idx)).getDimensions();
				std::cout<<" with a "<<dims.first<<"-dimensional archive with "<<dims.second<<" dimensions resulting in size "<< (uint64_t)std::pow(dims.first, dims.second);
				std::cout<<" Used range are [0; ";
				for(size_t i = 0; i < allArchiveValues.at(idx).size() - 1; i++){
					double value = round(allArchiveValues.at(idx)[i] * 100) / 100;
					std::cout<<value << "], ["<<value<<"; ";
				} 
				std::cout<<round(allArchiveValues.at(idx)[allArchiveValues.at(idx).size() - 1]*100)/100<<"]."<<std::endl;
			}
		}



	} else if(params.useTournamentSelection){
		std::cout<<" TOURNAMENT SELECTION"<<std::endl;
	} else {
		std::cout<<" STANDARD SELECTION"<<std::endl;
	}
	std::cout << "START MUJOCO APPLICATION WITH ENVIRONMENT "<< usecase << std::endl;
	std::cout << "NUMBER OF THREADS " << params.nbThreads << std::endl;



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

	// Create the archive CSV file
	for(auto descriptor: descriptorTypes){
		char archivePath[250];
		sprintf(archivePath, "%s/archive_%s.%" PRIu64 ".p%d.%s.csv", logsFolder, Descriptor::descriptorTypeToString(descriptor).c_str(), seed, indexParam, usecase);
		la.getMapElitesArchiveAt(descriptor).initCSVarchive(archivePath);
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
		
		for(auto descriptor: descriptorTypes){
			// Update the archive CSV file (TODO MAKE BETTER)
			char archivePath[250];
			sprintf(archivePath, "%s/archive_%s.%" PRIu64 ".p%d.%s.csv", logsFolder, Descriptor::descriptorTypeToString(descriptor).c_str(), seed, indexParam, usecase);
			la.getMapElitesArchiveAt(descriptor).updateCSVArchive(archivePath, i);
		}

		if(i % params.stepValidation == 0 && params.doValidation){

			for(size_t idx = 0; idx < descriptorTypes.size(); idx++){
				size_t index = 0;
				for (const auto& elem: la.getMapElitesArchiveAt(descriptorTypes.at(idx)).getAllArchive()) {
					if (elem.second != nullptr && la.getTPGGraph()->hasVertex(*elem.second)) {
						exportIndividual(elem.second, archiveDots.at(idx), archiveStats.at(idx), index, seed, indexParam, usecase,
										la.getTPGGraph(), dotExporter);
					}
					index++;
				}
			}
		}
	}

	
	//la.getTPGGraph()->clearProgramIntrons();


	if (!descriptorTypes.empty()) {
		for(size_t idx = 0; idx < descriptorTypes.size(); idx++){
			size_t index = 0;
			for (const auto& elem: la.getMapElitesArchiveAt(descriptorTypes.at(idx)).getAllArchive()) {
				if (elem.second != nullptr) {
					exportIndividual(elem.second, archiveDots.at(idx), archiveStats.at(idx), index, seed, indexParam, usecase,
									la.getTPGGraph(), dotExporter);
				}
				index++;
			}
		}

	} else {
		la.keepBestPolicy();
		const auto* best = la.getBestRoot().first;
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