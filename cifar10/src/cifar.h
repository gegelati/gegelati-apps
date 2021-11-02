#ifndef CIFAR_H
#define CIFAR_H

#include <random>
#include <gegelati.h>
#include "cifar_reader.h"


/**
* LearningEnvironment to train an agent to classify the CIFAR database.
*/
class CIFAR : public Learn::LearningEnvironment {
protected:
	/// CIFAR dataset for the training.
	static cifar::CIFAR10_dataset<std::vector, std::vector<uint8_t>, uint8_t> tmp_dataset;

	/// CIFAR dataset using doubles instead of uint8_t.
	static cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> dataset;

	/// CIFAR dataset using doubles instead of uint8_t.
	static cifar::CIFAR10_dataset<std::vector, std::vector<double>, uint8_t> subset;

	/// Current LearningMode of the LearningEnvironment.
	Learn::LearningMode currentMode;

	/// Randomness control
	Mutator::RNG rng;

	/// Current image provided to the LearningAgent
	Data::Array2DWrapper<double> currentImage;

	/// Current index of the image in the dataset.
	uint64_t currentIndex;

	/// Current class of the image in the dataset.
	uint64_t currentClass;

	/// Score of the generation
	int score;

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
	CIFAR();

	/// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;

	/// Inherited via LearningEnvironment
	virtual bool isCopyable() const override;

	/// Inherited via LearningEnvironment
	virtual LearningEnvironment* clone() const;

	/**
	* Get the score of the current evaluation session (i.e. since the last reset).
	*
	* For the CIFAR LearningEnvironment, the score is computed as follows:
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

	/**
	* \brief Retrieve the label of the current image.
	*
	* Should not be used to cheat.
	*/
	uint8_t getCurrentImageLabel();

	/**
	* \brief Function printing the statistics of the classification.
	*
	* \param[in] result the Map containing the list of roots within a TPGGraph,
	* with their score in ascending order.
	*/
	void printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot);

	/**
	* \brief turns the 3 R-G-B images in a single RGB image
	**/
	void processDataset();

	/**
	* \brief Updates the subset for the training.
	* A ratio of samples are deleted and the rest are kept in the vector 
	**/
	void updateSubset();
	};
#endif
