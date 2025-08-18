#include <random>
#include <inttypes.h>

#include "mnist.h"

mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> MNIST::dataset(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION));

void MNIST::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_images : this->dataset.test_images;

	// Select the image 
	// If the mode is training or validation
	if (this->currentMode == Learn::LearningMode::TRAINING || this->currentMode == Learn::LearningMode::VALIDATION) {
		// Select a random image index
		this->currentIndex = rng.getUnsignedInt64(0, dataSource.size() - 1);
	}
	else {
		// If the mode is TESTING, just go to the next image
		this->currentIndex = (this->currentIndex + 1) % dataSource.size();
	}

	// Load the image in the dataSource
	for (uint64_t pxlIndex = 0; pxlIndex < 28 * 28; pxlIndex++) {
		this->currentImage.setPointer(&dataSource.at(this->currentIndex));
	}

	// Keep current label too.
	this->currentClass = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_labels.at(this->currentIndex) : this->dataset.test_labels.at(this->currentIndex);

}

MNIST::MNIST() : ClassificationLearningEnvironment(10), currentImage(28, 28)
{
	// Fill shared dataset dataset(mnist::read_dataset<std::vector, std::vector, double, uint8_t>(MNIST_DATA_LOCATION))
	if (MNIST::dataset.training_labels.size() != 0) {
		std::cout << "Nbr of training images = " << dataset.training_images.size() << std::endl;
		std::cout << "Nbr of training labels = " << dataset.training_labels.size() << std::endl;
		std::cout << "Nbr of test images = " << dataset.test_images.size() << std::endl;
		std::cout << "Nbr of test labels = " << dataset.test_labels.size() << std::endl;
	}
	else {
		throw std::runtime_error("Initialization of MNIST databased failed.");
	}
}

void MNIST::doAction(double actionID)
{
	// Call to devault method to increment classificationTable
	ClassificationLearningEnvironment::doAction(actionID);

	this->changeCurrentImage();
}

void MNIST::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber)
{
	// Reset the classificationTable
	ClassificationLearningEnvironment::reset(seed);

	this->currentMode = mode;
	this->rng.setSeed(seed);

	// Reset at -1 so that in TESTING mode, first value tested is 0.
	this->currentIndex = -1;
	this->changeCurrentImage();
}

std::vector<std::reference_wrapper<const Data::DataHandler>> MNIST::getDataSources()
{
	std::vector<std::reference_wrapper<const Data::DataHandler>> res = { currentImage };

	return res;
}

bool MNIST::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* MNIST::clone() const
{
	return new MNIST(*this);
}

double MNIST::getScore() const
{
	// Return the default classification score
	return ClassificationLearningEnvironment::getScore();
}

bool MNIST::isTerminal() const
{
	return false;
}

uint8_t MNIST::getCurrentImageLabel()
{
	return (uint8_t)this->currentClass;
}

void MNIST::printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot) {
	// Print table of classif of the best
	TPG::TPGExecutionEngine tee(env, NULL);

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
		const TPG::TPGAction* action = (const TPG::TPGAction*)path.first.back();
		uint8_t actionID = (uint8_t)action->getActionID();

		// Increment table
		classifTable[currentLabel][actionID]++;

		// Do action (to trigger image update)
		this->doAction((double)action->getActionID());
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
