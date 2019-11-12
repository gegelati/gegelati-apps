#include <random>

#include <string>
#include <iostream>
#include "cicids.h"
#include <fstream>


void CICIDS::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::string>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->training_challenges : this->eval_challenges;

	// Select a random Challenge index
	std::uniform_int_distribution<uint64_t> distribution(0, dataSource.size() - 1);
	uint64_t challengeIndex = distribution(this->engine);

	// Load the Challenge in the dataSource
	/*for (uint64_t pxlIndex = 0; pxlIndex < 28 * 28; pxlIndex++) {
		this->currentChallenge.setDataAt(typeid(PrimitiveType<double>), pxlIndex, dataSource.at(imgIndex).at(pxlIndex));
	}*/
	getChallengeAt(challengeIndex);

	// Keep current label too.
	currentLabel = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->training_labels.at(challengeIndex) : this->eval_labels.at(challengeIndex);

}

CICIDS::CICIDS() : LearningEnvironment(11),nbCorrectGuesses{ 0 }, nbIncorrectGuesses{ 0 }, nbNoGuesses{ 0 }, currentChallenge(28 * 28), currentLabel{ 0 }, nbChallenges(100000)
{
	fname = "/home/nsourbie/Documents/gegelati-apps/cicids/dat/port_scan.csv";
	extractDatabase();
	std::cout << "Nbr of training images = " << training_challenges.size() << std::endl;
	std::cout << "Nbr of training labels = " << training_labels.size() << std::endl;
	std::cout << "Nbr of test images = " << eval_challenges.size() << std::endl;
	std::cout << "Nbr of test labels = " << eval_labels.size() << std::endl;
}

void CICIDS::doAction(uint64_t actionID)
{
	// If the action is 10 -> no guess was made by the network.
	if (actionID == 10) {
		this->nbNoGuesses++;
	}
	// An action has been done.
	// Check if the given action corresponds to the image label.
	else if (actionID == this->currentLabel) {
		this->nbCorrectGuesses++;
	}
	else {
		nbIncorrectGuesses++;
	}

	this->changeCurrentImage();


}

void CICIDS::reset(size_t seed, Learn::LearningMode mode)
{
	this->currentMode = mode;
	this->engine.seed(seed);
	this->nbCorrectGuesses = 0;
	this->nbIncorrectGuesses = 0;
	this->nbNoGuesses = 0;

	this->changeCurrentImage();
}

std::vector<std::reference_wrapper<DataHandlers::DataHandler>> CICIDS::getDataSources()
{
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> res = { currentChallenge };

	return res;
}

double CICIDS::getScore() const
{
	uint64_t totalNbGuesses = this->nbCorrectGuesses + this->nbIncorrectGuesses + this->nbNoGuesses;
	return (double)this->nbCorrectGuesses / (double)totalNbGuesses;
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
			classNames.insert({label,++labelIdx});
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
	int trainingC = nbChallenges/1.4;
	int evalC = trainingC * 0.3;
	int validC = trainingC * 0.1;

	//skip first line;
	getline(fin, line);

	//get string at end of line
	while(getline(fin, line) && count++ < trainingC)
	{
		int pos = line.rfind(",");
		training_labels.push_back(classNames.find(line.substr(pos+1))->second);
		line.erase(line.begin() + pos, line.end());
		this->training_challenges.push_back(line);	
	}
	count = 0;
	while(getline(fin, line) && count++ < evalC)
	{
		int pos = line.rfind(",");
		eval_labels.push_back(classNames.find(line.substr(pos+1))->second);
		line.erase(line.begin() + pos, line.end());
		this->eval_challenges.push_back(line);		
	}
	count = 0;
	while(getline(fin, line) && count++ < validC)
	{
		int pos = line.rfind(",");
		validation_labels.push_back(classNames.find(line.substr(pos+1))->second);
		line.erase(line.begin() + pos, line.end());
		this->validation_challenges.push_back(line);		
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
			this->currentChallenge.setDataAt(typeid(PrimitiveType<double>), i, std::nan(""));
		}
		else if(temp.compare("Infinity") == 0)
		{
			this->currentChallenge.setDataAt(typeid(PrimitiveType<double>), i, std::numeric_limits<double>::infinity());
		}
		else
		{
			tmpd = std::stof(temp);	
			this->currentChallenge.setDataAt(typeid(PrimitiveType<double>), i, tmpd);
		}
		pos1 = pos2 + 1;
		pos2 = line.find(",", pos1);
	}
}
