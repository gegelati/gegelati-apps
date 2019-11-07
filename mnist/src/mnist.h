#ifndef MNIST_H
#define MNIST_H

#include <random>

#include <gegelati.h>

#include "mnist_reader/mnist_reader.hpp"

/**
* LearningEnvironment to train an agent to classify the MNIST database.
*/
class MNIST : public Learn::LearningEnvironment {
protected:
	/// MNIST dataset for the training.
	mnist::MNIST_dataset<std::vector, std::vector<double>, uint8_t> dataset;

	/// Current LearningMode of the LearningEnvironment.
	Learn::LearningMode currentMode;

	/// Randomness control
	std::mt19937_64 engine;

	/// Number of correct guesses since the last reset.
	uint64_t nbCorrectGuesses;

	/// Number of incorrect guesses since the last reset.
	uint64_t nbIncorrectGuesses;

	/// Number of no guesses since the last reset.
	uint64_t nbNoGuesses;

	/// 
	DataHandlers::PrimitiveTypeArray<double> currentImage;

	/// Label of the current image
	uint8_t currentLabel;

	/**
	* \brief Change the image currently available in the dataSources of the LearningEnvironment.
	*
	* A random image from the dataset for the current mode is selected.
	*
	*/
	void changeCurrentImage();

public:

	/**
	* \brief Constructor.
	*
	* Loads the dataset on construction. Sets the LearningMode to TRAINING.
	* Sets the score to 0.
	*/
	MNIST();

	/// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<DataHandlers::DataHandler>> getDataSources() override;

	/**
	* Get the score of the current evaluation session (i.e. since the last reset).
	*
	* For the MNIST LearningEnvironment, the score is computed as follows:
	* - Score is incremented by 1.0 for a correct classification.
	* - Score is decremented by 0.25 for an incorrect classification.
	* - Score is left unchanged for action ID 10 which corresponds to an
	*   absence of guess from the agent.
	*
	* To be used in a viable learning process, score can be compared only on
	* agent evaluated on the same number of samples.
	*/
	virtual double getScore() const override;

	/**
	* \brief This classification LearningEnvironment will never reach a
	* terminal state.
	*
	* \return false.
	*/
	virtual bool isTerminal() const override;
};

#endif