#ifndef MNIST_H
#define MNIST_H

#include <random>

#include <gegelati.h>


/**
* LearningEnvironment to train an agent to classify the MNIST database.
*/
class MNIST : public Learn::LearningEnvironment {
protected:
	

public:
	/// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<DataHandlers::DataHandler>> getDataSources() override;

	/// Inherited via LearningEnvironment
	virtual double getScore() const override;

	/// Inherited via LearningEnvironment
	virtual bool isTerminal() const override;
};

#endif