/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2019 - 2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2019 - 2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

#ifndef STICK_GAME_ADVERSARIAL_H
#define STICK_GAME_ADVERSARIAL_H

#include "data/primitiveTypeArray.h"
#include "mutator/rng.h"

#include "learn/adversarialLearningEnvironment.h"

/**
 * Play the stick game against another agent
 */
class StickGameAdversarial : public Learn::AdversarialLearningEnvironment
{
  protected:
    /// During a game, number of remaining sticks.
    Data::PrimitiveTypeArray<int> remainingSticks;

    /// This source of data give useful numbers for helping undertanding the
    /// game.
    Data::PrimitiveTypeArray<int> hints;

    /// Did the first player win or loose
    bool firstPlayerWon;

    /// Did the first player attempt a forbidden move (i.e. removing more sticks
    /// than available)
    bool forbiddenMoveFirstPlayer;

    /// Same as forbiddenMoveFirstPlayer but for second player
    bool forbiddenMoveSecondPlayer;

    /// Randomness control
    Mutator::RNG rng;

    /// Simple turn control, first player plays in even currentTurn and second in odd
    int currentTurn;

    /// Error rate of the opponent algo.
    int errorRate = 4; // error will (potentially) be made 1/errorRate of the time

    /// In case the goal is not to do some adversarial learning, the opponent can
    /// be random
    bool isSecondPlayerRandom;

    /// Makes the random player play this turn
    void randomPlay();

  public:


    /**
     * Constructor.
     */
    StickGameAdversarial(bool isSecondPlayerRandom = true)
        : AdversarialLearningEnvironment(3), remainingSticks(1), hints(4),
        isSecondPlayerRandom(isSecondPlayerRandom)
    {
        this->reset(0);
        // Set hints
        this->hints.setDataAt(typeid(int), 0, 1);
        this->hints.setDataAt(typeid(int), 1, 2);
        this->hints.setDataAt(typeid(int), 2, 3);
        this->hints.setDataAt(typeid(int), 3, 4);
    };

    /**
    * \brief Copy constructor for the StickGame.
    *
    * Default copy constructor since all attributes are trivially copyable.
    */
    StickGameAdversarial(const StickGameAdversarial& other) = default;

    /// Destructor
    ~StickGameAdversarial(){};

    // Inherited via LearningEnvironment
    virtual bool isCopyable() const override;

    // Inherited via LearningEnvironment
    virtual LearningEnvironment* clone() const override;

    // Inherited via LearningEnvironment
    virtual void doAction(uint64_t actionID) override;

    // Inherited via LearningEnvironment
    virtual void reset(
        size_t seed = 0,
        Learn::LearningMode mode = Learn::LearningMode::TRAINING,
        uint16_t iterationNumber = 0,
        uint64_t generationNumber = 0) override;

    // Inherited via LearningEnvironment
    virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
    getDataSources() override;

    /**
    * Inherited from LearningEnvironment.
    *
    * In this game, the score is 0 during the Game. At the end of the game
    * the score is 1 if the agent won, 0 if the agent lost, and -1 if the agent
    * took more sticks than available at the end of the game. In this case, the
    * other is considered as winner (and he gets 1).
    *
    * @return scores of both players.
    */
    virtual std::shared_ptr<Learn::AdversarialEvaluationResult> getScores()
        const override;

    // Inherited via LearningEnvironment
    virtual bool isTerminal() const override;

    /// Just returns the number of remaining sticks as a string
    std::string toString() const;
};

#endif
