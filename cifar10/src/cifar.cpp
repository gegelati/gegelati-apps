#include <random>
#include <inttypes.h>
#include <typeinfo>
#include "cifar.h"

#define RATIO_NEW_SAMPLES 0.5

cifar::CIFAR10_dataset<std::vector, std::vector<uint8_t>, uint8_t> CIFAR::tmp_dataset(cifar::read_dataset());
cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> CIFAR::dataset;
cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> CIFAR::subset;

void CIFAR::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->subset.training_images : this->dataset.test_images;

	this->currentIndex = (this->currentMode == Learn::LearningMode::TRAINING) ?
	(this->currentIndex + 1) % dataSource.size() : this->rng.getUnsignedInt64(0, dataSource.size()-1);

	// Load the image in the dataSource
	this->currentImage.setPointer(&dataSource.at(this->currentIndex));
	this->currentClass = (this->currentMode == Learn::LearningMode::TRAINING) ?
	this->subset.training_labels.at(this->currentIndex) : this->dataset.test_labels.at(this->currentIndex);
}

CIFAR::CIFAR() : LearningEnvironment(10), currentImage(32, 32), currentClass(0), score(0)
{
	// Fill shared tmp_dataset tmp_dataset(cifar::read_dataset<std::vector, std::vector, double, uint8_t>(CIFAR_DATA_LOCATION))
	if (CIFAR::tmp_dataset.training_labels.size() != 0) {
		std::cout << "Nbr of training images = " << tmp_dataset.training_images.size() << std::endl;
		std::cout << "Nbr of training labels = " << tmp_dataset.training_labels.size() << std::endl;
		std::cout << "Nbr of test images = " << tmp_dataset.test_images.size() << std::endl;
		std::cout << "Nbr of test labels = " << tmp_dataset.test_labels.size() << std::endl;
	}
	else {
		throw std::runtime_error("Initialization of CIFAR databased failed.");
	}
	processDataset();
}

void CIFAR::doAction(uint64_t actionID)
{
	// Call to devault method to increment classificationTable
	if(this->currentClass == actionID)
	{
		score += 1;
	}
	LearningEnvironment::doAction(actionID);
	this->changeCurrentImage();
}

void CIFAR::reset(size_t seed, Learn::LearningMode mode)
{
	// Reset the classificationTable
	//ClassificationLearningEnvironment::reset(seed);

	this->currentMode = mode;
	this->rng.setSeed(seed);
	this->score = 0;
	
	// Reset at -1 so that in TESTING mode, first value tested is 0.
	this->currentIndex = -1;
	this->changeCurrentImage();
}

std::vector<std::reference_wrapper<const Data::DataHandler>> CIFAR::getDataSources()
{
	std::vector<std::reference_wrapper<const Data::DataHandler>> res = { currentImage };

	return res;
}

bool CIFAR::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* CIFAR::clone() const
{
	return new CIFAR(*this);
}

double CIFAR::getScore() const
{
	return this->score/120.0;
	//return ClassificationLearningEnvironment::getScore();
}

bool CIFAR::isTerminal() const
{
	return false;
}

uint8_t CIFAR::getCurrentImageLabel()
{
	return (uint8_t)this->currentClass;
}

void CIFAR::printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(env, NULL);

	// Change the MODE of cifar
	this->reset(0, Learn::LearningMode::TESTING);

	// Fill the table
	uint64_t classifTable[10][10] = { 0 };
	uint64_t nbPerClass[10] = { 0 };

	const int TOTAL_NB_IMAGE = 10000;
	for (int nbImage = 0; nbImage < TOTAL_NB_IMAGE; nbImage++) {
		// Get answer
		uint8_t currentLabel = this->getCurrentImageLabel();
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
	for (int i = 0; i < 10; i++) {
		printf("%d\t", i);
	}
	printf("Nb\n");
	for (int i = 0; i < 10; i++) {
		printf("%d\t", i);
		for (int j = 0; j < 10; j++) {
			if (i == j) {
				printf("\033[0;32m");
			}
			printf("%2.1f\t", 100.0 * (double)classifTable[i][j] / (double)nbPerClass[i]);
			if (i == j) {
				printf("\033[0m");
			}
		}
		printf("%4" PRIu64 "\n", nbPerClass[i]);
	}
	std::cout << std::endl;
}

void CIFAR::processDataset()
{
		auto trainingSet = this->tmp_dataset.training_images;
		auto testSet = this->tmp_dataset.test_images;

		for(int i = 0; i < trainingSet.size(); ++i)
		{
			auto image = trainingSet.at(i);
			std::vector<double> tmpimage;
			for(int j = 0; j < 32; ++j)
			{
				for(int k = 0; k < 32; ++k)
				{
					int pxlIndex = 32*j+k;
					int32_t value = trainingSet.at(i).at(pxlIndex)<<16 | 
					trainingSet.at(i).at(32*32+pxlIndex)<<8 | 
					trainingSet.at(i).at(32*32*2+pxlIndex);
					//double * ptrDbl = (double *)&value;
					//tmpimage.push_back(*ptrDbl);
					tmpimage.push_back(value);
				}
			}
			this->dataset.training_images.push_back(tmpimage);	
			this->dataset.training_labels.push_back(this->tmp_dataset.training_labels.at(i));
		}

		for(int i = 0; i < testSet.size(); ++i)
		{
			auto image = testSet.at(i);
			std::vector<double> tmpimage;
			for(int j = 0; j < 32; ++j)
			{
				for(int k = 0; k < 32; ++k)
				{
					int pxlIndex = 32*j+k;
					int64_t value = testSet.at(i).at(pxlIndex)<<16 | 
					testSet.at(i).at(32*32+pxlIndex)<<8 | 
					testSet.at(i).at(32*32*2+pxlIndex);
					//double * ptrDbl = (double *)&value;
					//tmpimage.push_back(*ptrDbl);
					tmpimage.push_back(value);
				}
			}
			this->dataset.test_images.push_back(tmpimage);	
			this->dataset.test_labels.push_back(this->tmp_dataset.test_labels.at(i));
		}

		this->tmp_dataset.training_images.empty();
		this->tmp_dataset.training_labels.empty();
		this->tmp_dataset.test_images.empty();
		this->tmp_dataset.test_labels.empty();

		Learn::LearningParameters params;
		File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
		//init subset
		for(int i = 0; i < 10; ++i)
		{
			int count = 0;
			while (count < params.maxNbActionsPerEval/10)
			{
				//generate a random number 
				int32_t index = rng.getUnsignedInt64(0, this->dataset.training_images.size() - 1);
				//we want the same number of samples of the same class in the dataset
				if(this->dataset.training_labels.at(index) == i)
				{
					this->subset.training_images.push_back(this->dataset.training_images.at(index));
					this->subset.training_labels.push_back(this->dataset.training_labels.at(index));
					++count;
				}
				
			}
		}
}

void CIFAR::updateSubset()
{
		Learn::LearningParameters params;
		File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);

		int to_delete = params.maxNbActionsPerEval*RATIO_NEW_SAMPLES;

		//delete random sambles in the subset
		for(int i = 0; i < to_delete ; ++i)
		{
			int32_t index = rng.getUnsignedInt64(0, this->subset.training_images.size() - 1);

			this->subset.training_images.erase(this->subset.training_images.begin() + index);
			this->subset.training_labels.erase(this->subset.training_labels.begin() + index);
		}

		//add random samples to the training set
		int count_classes[10]={0}; //count per class occurances in what's left of the subset
		for (int i = 0; i < this->subset.training_images.size(); ++i)
		{
			++count_classes[this->subset.training_labels.at(i)];
		}

		for(int i = 0; i < 10; ++i)
		{
			int count = count_classes[i];
			while (count < params.maxNbActionsPerEval/10)
			{
				//generate a random number 
				int32_t index = rng.getUnsignedInt64(0, this->dataset.training_images.size() - 1);
				//we want the same number of samples of the same class in the dataset
				if(this->dataset.training_labels.at(index) == i)
				{
					this->subset.training_images.push_back(this->dataset.training_images.at(index));
					this->subset.training_labels.push_back(this->dataset.training_labels.at(index));
					++count;
				}
				
			}
		}
}