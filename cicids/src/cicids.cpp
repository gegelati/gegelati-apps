#include <random>

#include <string>
#include <iostream>
#include <pcap.h>
#include "cicids.h"
#include <fstream>


void CICIDS::changeCurrentChallenge()
{
	// Get the container for the current mode.
	std::vector<std::string>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->training_challenges : this->eval_challenges;

	// Select a random Challenge index
	uint64_t challengeIndex = rng.getUnsignedInt64(0,dataSource.size()-1);

	getChallengeAt(challengeIndex);
	
	// Keep current label too.
	currentLabel = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->training_labels.at(challengeIndex) : this->eval_labels.at(challengeIndex);
}

CICIDS::CICIDS() : ClassificationLearningEnvironment(15),nbCorrectGuesses{ 0 }, nbIncorrectGuesses{ 0 }, nbNoGuesses{ 0 }, currentChallenge(78), currentLabel{ 0 }, nbChallenges(1000000)
{
	fname = "/home/nsourbie/Documents/gegelati-apps/cicids/dat/dataset.csv";
	extractDatabase();
	std::cout << "Nbr of training images = " << training_challenges.size() << std::endl;
	std::cout << "Nbr of training labels = " << training_labels.size() << std::endl;
	std::cout << "Nbr of test images = " << eval_challenges.size() << std::endl;
	std::cout << "Nbr of test labels = " << eval_labels.size() << std::endl;
}

int count_seek = 0;
void CICIDS::doAction(uint64_t actionID)
{
	ClassificationLearningEnvironment::doAction(actionID);
	this->changeCurrentChallenge();
}

void CICIDS::reset(size_t seed, Learn::LearningMode mode)
{
	this->currentMode = mode;
	this->rng.setSeed(seed);
	this->nbCorrectGuesses = 0;
	this->nbIncorrectGuesses = 0;
	this->nbNoGuesses = 0;
	this->changeCurrentChallenge();
	count_seek = 0;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> CICIDS::getDataSources()
{
	std::vector<std::reference_wrapper<const Data::DataHandler>> res = {currentChallenge};

	return res;
}

bool CICIDS::isCopyable() const 
{
	return false;
}

Learn::LearningEnvironment* CICIDS::clone() const
{
	return new CICIDS(*this);
}

double CICIDS::getScore() const
{
	return ClassificationLearningEnvironment::getScore();
}

bool CICIDS::isTerminal() const
{
	return false;
}

void CICIDS::getClassNames()
{
	//open file
	std::ifstream fin(fname);

	std::string line = "";
	std::string label = "";
	int labelIdx = 0;
	//skip first line;
	getline(fin, line);
	//get string at end of line
	while(getline(fin, line))
	{
		int pos = line.rfind(",");
		label = line.substr(pos+1);
		if(label.compare("") == 0)
			continue;
		//add string into className with int
		if(classNames.find(label) == classNames.end())
		{
			classNames.insert({label,labelIdx++});
			this->datasetSize++;
		}
	}

	//close file
	fin.close();
}


void CICIDS::extractDatabase()
{

	getClassNames();

	//open file
	std::ifstream fin(fname);
	std::string line = "";
	int count = 0;

	//skip first line;
	getline(fin, line);

	while(count++ < nbChallenges)
	{
		getline(fin, line);
		uint64_t prob = rng.getUnsignedInt64(0,100);
		if(prob < 25) 					//we randomly pick 25 % of the challenges for the evaluation
		{
			int pos = line.rfind(",");
			eval_labels.push_back(classNames.find(line.substr(pos+1))->second);
			line.erase(line.begin() + pos, line.end());
			this->eval_challenges.push_back(line);
		}
		else
		{
			int pos = line.rfind(",");
			training_labels.push_back(classNames.find(line.substr(pos+1))->second);
			line.erase(line.begin() + pos, line.end());
			this->training_challenges.push_back(line);
		}
	}
	
	//close file
	fin.close();
}


void CICIDS::getChallengeAt(uint32_t index)
{
	currentChallenge.resetData();
	
	std::string line = "";

	std::string label = "";

	if(this->currentMode == Learn::LearningMode::TRAINING)
	{
		line = this->training_challenges[index];
	}
	else
	{
		line = this->eval_challenges[index];
	}

	int pos1 = 0;
	int pos2 = line.find(",",0);
	std::string temp;
	double tmpd;

	for(int i = 0; i < stimulusSize; i++)
	{
		temp = line.substr(pos1, pos2-pos1);
		if(temp.compare("NaN") == 0)
		{
			this->currentChallenge.setDataAt(typeid(double), i, std::nan(""));
		}
		else if(temp.compare("Infinity") == 0)
		{
			this->currentChallenge.setDataAt(typeid(double), i, std::numeric_limits<double>::infinity());
		}
		else
		{
			tmpd = std::stof(temp);	
			this->currentChallenge.setDataAt(typeid(double), i, tmpd);
		}
		pos1 = pos2 + 1;
		pos2 = line.find(",", pos1);
	}
}


void CICIDS::printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(env, NULL);

	// Change the MODE of mnist
	this->reset(0, Learn::LearningMode::TESTING);

	// Fill the table
	uint64_t classifTable[15][15] = { 0 };
	uint64_t nbPerClass[15] = { 0 };

	const int TOTAL_NB_IMAGE = 2000;
	for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
		// Get answer
		uint8_t currentLabel = this->currentLabel;
		nbPerClass[currentLabel]++;
		
		// Execute
		auto path = tee.executeFromRoot(*bestRoot);
		const TPG::TPGAction* action = (const TPG::TPGAction*)path.at(path.size() - 1);
		uint8_t actionID = (uint8_t)action->getActionID();
		// Increment table
		classifTable[currentLabel][actionID]++;
		// Do action (to trigger image update)
		this->doAction(action->getActionID());
	}

	// Print the table
	printf("\t");
	for (int i = 0; i < 15; i++) {
		printf("%d\t", i);
	}
	printf("Nb\n");
	for (int i = 0; i < 15; i++) {
		printf("%d\t", i);
		for (int j = 0; j < 15; j++) {
			if (i == j) {
				printf("\033[0;32m");
			}
			//printf("%2.1f\t", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
			printf("%2.1f\t",(double)classifTable[i][j]);
			if (i == j) {
				printf("\033[0m");
			}
		}
		printf("%4" PRIu64 "\n", nbPerClass[i]);
	}
	std::cout << std::endl;
}
