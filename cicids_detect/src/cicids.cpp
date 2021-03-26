#include <random>

#include <string>
#include <iostream>
#include <pcap.h>
#include "cicids.h"
#include <fstream>


void CICIDS::changeCurrentChallenge()
{
	static int chall_train_idx = 0;
	static int chall_eval_idx = 0;
	static int count = 0;
	uint64_t chall_idx;
	uint32_t alpha = 833044;
	
	if(nb_gen < 100)
	{
		chall_idx = (this->currentMode == Learn::LearningMode::TRAINING) ?
			chall_train_idx++%training_labels.size()://alpha : 
			chall_eval_idx++%(eval_labels.size()-2);
	}
	else{
		chall_idx = (this->currentMode == Learn::LearningMode::TRAINING) ?
			alpha + (chall_train_idx++%(training_labels.size()-alpha)) : 
			chall_eval_idx++%(eval_labels.size()-2);
	}
	
	currentLabel = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->training_labels.at(chall_idx) : this->eval_labels.at(chall_idx);
	/*static int first = 1 ;
	if(first && currentLabel == 5)
	{
		first = 0;
		std::cout << std::endl << chall_idx << std::endl << std::endl; 
	}*/	
	getChallengeAt(chall_idx);
	chall_idx = (this->currentMode == Learn::LearningMode::TRAINING) ?
	chall_train_idx++%(training_labels.size()-2) : 
	chall_eval_idx++%(eval_labels.size()-2);

	static bool exclude = false;
	if(nb_gen == 100 && !exclude)
	{
		exclude = true;
		printf("\ntraining on new dataset\n");
	}
	count ++;
}

CICIDS::CICIDS() : LearningEnvironment(2),classificationTable(15, std::vector<uint64_t>(15, 0)), currentChallenge(78), currentLabel{ 0 }, nbChallenges(1600000), stimulusSize(78), nb_gen(0)
{
	fname = "../dat/dataset.csv";
	extractDatabase();
	std::cout << "Nbr of training images = " << training_challenges.size() << std::endl;
	std::cout << "Nbr of training labels = " << training_labels.size() << std::endl;
	std::cout << "Nbr of test images = " << eval_challenges.size() << std::endl;
	std::cout << "Nbr of test labels = " << eval_labels.size() << std::endl;
}

int count_seek = 0;
void CICIDS::doAction(uint64_t actionID)
{
        // Base method
        LearningEnvironment::doAction(actionID);

        // Classification table update
      	const int lbl = (int)this->currentLabel;
      	const int act = (int)actionID;
      	
      	
		//if(act == 1)
			//std::cout << "action : " << act << " on sample labelled : " << lbl << std::endl;
      	if(lbl != 0 && act == 1)  //tp
      	{
      	      	this->classificationTable.at(lbl).at(lbl) += 1;
      	}
      	else if(lbl == 0 && act == 1) //fp
      	{
      	      	this->classificationTable.at(0).at(1) += 5;//penlaty on the false pos
      	}
      	else if(lbl != 0 && act == 0) //fn
      	{
      	      	this->classificationTable.at(lbl).at(0) += 1;
      	}
      	else if(lbl == 0 && act == 0) //tn
      	{
      	      	this->classificationTable.at(0).at(0) += 1;
      	}

	changeCurrentChallenge();
}

void CICIDS::reset(size_t seed, Learn::LearningMode mode)
{
	this->currentMode = mode;
	this->rng.setSeed(seed);
	this->changeCurrentChallenge();
	count_seek = 0;
	// reset scores to 0 in classification table
    	for (std::vector<uint64_t>& perClass : this->classificationTable) {
        	for (uint64_t& score : perClass) {
        		score = 0;
        	}
    	}
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
    // Compute the average f1 score over all classes
    // (chosen instead of the global f1 score as it gives an equal weight to
    // the f1 score of each class, no matter its ratio within the observed
    // population)

    int nb_classes_seen = 0;
    for(int i = 0; i < classificationTable.size(); i++)
    {
    	for (int j = 0; j < classificationTable.size();j++)
    	{	
    	    if(classificationTable.at(i).at(j)!=0)
    	    {
    	    	++nb_classes_seen;
    	    	break;
    	    }
   	}
    }
    if(nb_classes_seen == 1)
    {
    	nb_classes_seen = 3; // penalties if we only saw one class (most likely BENIGN..)
    }
    
    //we want to reward detection even if classif is wrong.
    double avg_recall = 0;
    int first_col = 0;
    for(int j = 0; j < classificationTable.size(); j++)
    {
	first_col += classificationTable.at(j).at(0);
    }
    avg_recall += first_col != 0 ? 
		(classificationTable.at(0).at(0))/(double)first_col : 0; 	//get rid of false positive and that's it
		
    for(int i = 1; i < classificationTable.size(); i++)
    {
    	int tot = 0;
	for(int j = 0; j < classificationTable.size(); j++)
	{
	    tot += classificationTable.at(j).at(i);
	}
	avg_recall += tot != 0 ? 
		(tot-classificationTable.at(0).at(i))/(double)tot : 0; 	//get rid of false positive and that's it
    }
    avg_recall /= nb_classes_seen;
     
    double avg_precision = 0;
    int first_line = 0;
    for(int j = 0; j < classificationTable.size(); j++)
    {
	first_line += classificationTable.at(0).at(j);
    }
    avg_precision += first_line != 0 ? 
		(classificationTable.at(0).at(0))/(double)first_line : 0; 	//get rid of false positive and that's it
		
    for(int i = 1; i < classificationTable.size(); i++)
    {
    	int tot = 0;
	for(int j = 0; j < classificationTable.size(); j++)
	{
	    tot += classificationTable.at(i).at(j);
	}
	avg_precision += tot != 0 ? 
	(tot-classificationTable.at(i).at(0))/(double)tot : 0;			//get rid of false negative and that's it
    }
    avg_precision /= nb_classes_seen;
    
    if(avg_precision != 0)
    	return 2 * (avg_precision * avg_recall) / (avg_precision + avg_recall);
    return 0;
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
	unsigned int count = 0;
	//skip first line;
	getline(fin, line);

	while(count++ < nbChallenges)
	{
		getline(fin, line);
		int pos = line.rfind(",");
		unsigned int label = (classNames.find(line.substr(pos+1))->second);
		uint64_t prob = rng.getUnsignedInt64(0,100);
		if(label != 0 )
		{
			if(prob < 60) 					//we randomly pick 25 % of the challenges for the evaluation
			{
				eval_labels.push_back(label);
				line.erase(line.begin() + pos, line.end());
				this->eval_challenges.push_back(line);
			}
			else
			{
				training_labels.push_back(label);
				line.erase(line.begin() + pos, line.end());
				this->training_challenges.push_back(line);
			}
		}
		else
		{
			if(prob < 20) 					//we randomly pick 25 % of the challenges for the evaluation
			{
				eval_labels.push_back(label);
				line.erase(line.begin() + pos, line.end());
				this->eval_challenges.push_back(line);
			}
			else
			{
				training_labels.push_back(label);
				line.erase(line.begin() + pos, line.end());
				this->training_challenges.push_back(line);
			}
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
	
	std::cout << std::endl;

	// Fill the table
	uint64_t classifTable[15][15] = { 0 };
	uint64_t nbPerClass[15] = { 0 };

	const int TOTAL_NB_IMAGE = 283964;
	for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
		// Get answer
		int currentLabel = (int)this->currentLabel;
		nbPerClass[currentLabel]++;
		
		// Execute
		auto path = tee.executeFromRoot(*bestRoot);
		const TPG::TPGAction* action = (const TPG::TPGAction*)path.back();
		int actionID = (int)action->getActionID();
		
		if(currentLabel != 0 && actionID == 1)  //tp
	      	{
			classifTable[currentLabel][currentLabel]++;
	      	}
	      	else if(currentLabel == 0 && actionID == 1) //fp
	      	{
			classifTable[0][actionID]++;
	      	}
	      	else if(currentLabel != 0 && actionID == 0) //fn
	      	{
			classifTable[currentLabel][0]++;
	      	}
	      	else if(currentLabel == 0 && actionID == 0) //tn
	      	{
			classifTable[0][0]++;
	      	}

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
			if(nbPerClass[i]!=0)
				printf("%2.1f\t", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
			else
				printf("%2.1f\t", 0.0);	
			if (i == j) {
				printf("\033[0m");
			}
		}
		printf("%4" PRIu64 "\n", nbPerClass[i]);
	}
	std::cout << std::endl;
	
	int nb_classes_seen = 0;
        for(int i = 0; i < classificationTable.size(); i++)
        {
    	    for (int j = 0; j < classificationTable.size();j++)
    	    {	
    	        if(classificationTable.at(i).at(j)!=0)
    	        {
    	            ++nb_classes_seen;
    	    	    break;
    	        }
   	    }
        }

        double recall = 0;
        int first_col = 0;
        for(int j = 0; j < classificationTable.size(); j++)
    	{
		first_col += classificationTable.at(j).at(0);
    	}
    	recall += first_col != 0 ? 
		(classificationTable.at(0).at(0))/(double)first_col : 0; 	//get rid of false positive and that's it
        for(int i = 1; i < 15; i++)
        {
    	    int tot = 0;
	    for(int j = 0; j < 15; j++)
	    {
	        tot += classifTable[j][i];
	    }
	    recall += tot != 0 ? (tot-classifTable[0][i])/(double)tot : 0;
        }
        recall /= nb_classes_seen;
     
        double precision = 0;
        int first_line = 0;
    	for(int j = 0; j < classificationTable.size(); j++)
    	{
		first_line += classificationTable.at(0).at(j);
    	}
    	precision += first_line != 0 ? 
		(classificationTable.at(0).at(0))/(double)first_line : 0; 	//get rid of false positive and that's it
        for(int i = 1; i < 15; i++)
        {
    	    int tot = 0;
	    for(int j = 0; j < 15; j++)
	    {
	        tot += classifTable[i][j];
	    }
	    precision += tot != 0 ? (tot-classifTable[i][0])/(double)tot : 0;
        }
        precision /= nb_classes_seen;
        double f1 = (precision != 0) ? 2*(precision*recall)/(precision+recall) : 0.0;
        
        double accuracy = 0;
        int tp = 0;
        int tot = 0;
        for(int i = 0; i < 15; i++)
        {
	    for(int j = 0; j < 15; j++)
	    {
	        tot += classifTable[i][j];
	    }
	    tp += classifTable[i][i];
        }
        accuracy = (double)tp/(double)tot;
        
        printf("\taccuracy\tprecision\trecall\t\tf1\n");
        printf("\t%2.2f\t\t%2.2f\t\t%2.2f\t\t%2.2f\n",accuracy,precision,recall,f1);       
}

void CICIDS::incGen()
{
	nb_gen++;		
}
