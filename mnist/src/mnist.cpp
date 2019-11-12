#include <random>

#include "mnist.h"

void MNIST::changeCurrentImage()
{
	// Get the container for the current mode.
	std::vector<std::vector<double>>& dataSource = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_images : this->dataset.test_images;

	// Select a random image index
	std::uniform_int_distribution<uint64_t> distribution(0, dataSource.size() - 1);
	uint64_t imgIndex = distribution(this->engine);

	// Load the image in the dataSource
	for (uint64_t pxlIndex = 0; pxlIndex < 28 * 28; pxlIndex++) {
		this->currentImage.setDataAt(typeid(PrimitiveType<double>), pxlIndex, dataSource.at(imgIndex).at(pxlIndex));
	}

	// Keep current label too.
	currentLabel = (this->currentMode == Learn::LearningMode::TRAINING) ?
		this->dataset.training_labels.at(imgIndex) : this->dataset.test_labels.at(imgIndex);

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
