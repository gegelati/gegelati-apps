#include <random>

#include "mnist.h"

void MNIST::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_images : this->dataset.test_images;

	// Select the image 
	// If the mode is training or validation
	if (this->currentMode == Learn::LearningMode::TRAINING || this->currentMode == Learn::LearningMode::VALIDATION) {
		// Select a random image index
		std::uniform_int_distribution<uint64_t> distribution(0, dataSource.size() - 1);
		this->currentIndex = distribution(this->engine);
	}
	else {
		// If the mode is TESTING, just go to the next image
		this->currentIndex = (this->currentIndex + 1) % dataSource.size();
	}

	// Load the image in the dataSource
	for (uint64_t pxlIndex = 0; pxlIndex < 28 * 28; pxlIndex++) {
		this->currentImage.setDataAt(typeid(PrimitiveType<double>), pxlIndex, dataSource.at(this->currentIndex).at(pxlIndex));
	}

	// Keep current label too.
	currentLabel = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_labels.at(this->currentIndex) : this->dataset.test_labels.at(this->currentIndex);

}

MNIST::MNIST() : LearningEnvironment(10), dataset(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION)),
nbCorrectGuesses{ 0 }, nbIncorrectGuesses{ 0 }, nbNoGuesses{ 0 }, currentImage(28 * 28), currentLabel{ 0 }
{
	std::cout << "Nbr of training images = " << dataset.training_images.size() << std::endl;
	std::cout << "Nbr of training labels = " << dataset.training_labels.size() << std::endl;
	std::cout << "Nbr of test images = " << dataset.test_images.size() << std::endl;
	std::cout << "Nbr of test labels = " << dataset.test_labels.size() << std::endl;
}

void MNIST::doAction(uint64_t actionID)
{
	// An action has been done.
	// Check if the given action corresponds to the image label.
	if (actionID == this->currentLabel) {
		this->nbCorrectGuesses++;
	}
	else {
		nbIncorrectGuesses++;
	}

	this->changeCurrentImage();
}

void MNIST::reset(size_t seed, Learn::LearningMode mode)
{
	this->currentMode = mode;
	this->engine.seed(seed);
	this->nbCorrectGuesses = 0;
	this->nbIncorrectGuesses = 0;
	this->nbNoGuesses = 0;

	// Reset at -1 so that in TESTING mode, first value tested is 0.
	this->currentIndex = -1;
	this->changeCurrentImage();
}

std::vector<std::reference_wrapper<DataHandlers::DataHandler>> MNIST::getDataSources()
{
	std::vector<std::reference_wrapper<DataHandlers::DataHandler>> res = { currentImage };

	return res;
}

double MNIST::getScore() const
{
	uint64_t totalNbGuesses = this->nbCorrectGuesses + this->nbIncorrectGuesses;
	return (double)this->nbCorrectGuesses / (double)totalNbGuesses;
}

bool MNIST::isTerminal() const
{
	return false;
}

uint8_t MNIST::getCurrentImageLabel()
{
	return this->currentLabel;
}

void MNIST::printClassifStatsTable(std::multimap<double, const TPG::TPGVertex*>& result) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(NULL);

	// Get best root		
	auto iterResults = result.begin();
	std::advance(iterResults, result.size() - 1);
	auto bestRoot = iterResults->second;

	// Change the MODE of mnist
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
		printf("%lld\n", nbPerClass[i]);
	}
	std::cout << std::endl;
}
