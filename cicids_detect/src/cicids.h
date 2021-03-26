#ifndef CICIDS_H
#define CICIDS_H

#include <random>

#include <gegelati.h>

/**
* LearningEnvironment to train an agent to classify the CICIDS database.
*/
class CICIDS : public Learn::LearningEnvironment {
protected:
	/**
         * \brief 2D array storing for each class the guesses that were made by
         * the LearningAgent.
         *
         * For example classificationTable.at(x).at(y) represents the number of
         * times a LearningAgent guessed class y, for a data from class x since
         * the last reset.
         */
        std::vector<std::vector<uint64_t>> classificationTable;
        
	/// CICIDS dataset for the training.
	std::vector<std::string> training_challenges;

	/// CICIDS dataset for the evaluation
	std::vector<std::string> eval_challenges;

	/// CICIDS labels for the training.
	std::vector<uint8_t> training_labels;

	/// CICIDS labels for the evaluation
	std::vector<uint8_t> eval_labels;

	///name of the classes in the dataset
	std::map<std::string, int> classNames;

	/// Current LearningMode of the LearningEnvironment.
	Learn::LearningMode currentMode;

	/// Randomness control
	Mutator::RNG rng;

	/// the current log that we are observing
	Data::PrimitiveTypeArray<double> currentChallenge;

	/// Label of the current challenge
	uint8_t currentLabel;

	/// Number of challenges to run for training evaluation and validation 
	uint32_t nbChallenges;

	/// Number of available challenges in the dataset
	uint64_t datasetSize;

	/// Name of the pcap input file
	std::string fname;

	/// Size of the challenge
	uint16_t stimulusSize;
	
	/// Number of generations
	uint16_t nb_gen;


	/**
	* \brief Change the challenge currently available in the dataSources of the LearningEnvironment.
	*
	* A random challenge from the dataset for the current mode is selected.
	*
	*/
	void changeCurrentChallenge();

public:

	/**
	* \brief Constructor.
	*
	* Loads the dataset on construction. Sets the LearningMode to TRAINING.
	* Sets the score to 0.
	*/
	CICIDS();

	/// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<const Data::DataHandler>>  getDataSources() override;

	/// Inherited via LearningEnvironment
	virtual bool isCopyable() const override;
	
	/// Inherited via LearningEnvironment
	virtual LearningEnvironment* clone() const;
	
	/**
	* Get the score of the current evaluation session (i.e. since the last reset).
	*
	* For the CICIDS LearningEnvironment, the score is computed as follows:
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
	* \brief This function reads the dataset and get the number of classes 
	* and their names
	*/
	void getClassNames();

	/**
	* \brief This function extracts the amount of samples required from the input file
	* to fill training, evaluation and validation database
	*/
	void extractDatabase();

	/**
	* \brief this function converts a line from the dataset into a vector of floats
	*/
	void getChallengeAt(uint32_t index);
	
	/**
	* \brief Function printing the statistics of the classification.
	*
	* \param[in] result the Map containing the list of roots within a TPGGraph,
	* with their score in ascending order.
	*/
	void printClassifStatsTable(const Environment& env, const TPG::TPGVertex* bestRoot);
	
	/**
	* \brief incrments the current number of generations.
	*/
	void incGen();
};

#endif
