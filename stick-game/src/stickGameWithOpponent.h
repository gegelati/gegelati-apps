#ifndef STICK_GAME_WITH_OPPONENT_H
#define STICK_GAME_WITH_OPPONENT_H

#include <random>

#include <gegelati.h>


/**
* LearningEnvironment to play the stick game against a random player.
* The principle of the stick game is as follows. 21 sticks are layed out.
* Alternatively, each player plays a turn, and has to take 1, 2 or 3 sticks.
* The player taking the last stick on the table loses.
*
* In this LearningEnvironment, the trained agent plays against a "smart" algo
* that plays the optimal (winning) strategy, with a controlled error rate
* (defined as an ugly constant attribute).
*/
class StickGameWithOpponent : public Learn::LearningEnvironment {
protected:
	/// During a game, number of remaining sticks.
	DataHandlers::PrimitiveTypeArray<int> remainingSticks;

	/// This source of data give useful numbers for helping undertanding the game.
	DataHandlers::PrimitiveTypeArray<int> hints;

	/// Did the player win or lose at the end of a game.
	bool win;

	/// Did the player attempt a forbidden move (i.e. removing more sticks than available)
	bool forbiddenMove;

	/// Randomness control
	std::mt19937_64 engine;

	// Error rate of the opponent algo.
	int errorRate = 4; // error will (potentially) be made 1/errorRate of the time

public:
	/**
	* Constructor.
	*/
	StickGameWithOpponent() : LearningEnvironment(3), remainingSticks(1), hints(4), win{ false }{
		this->reset(0);
		// Set hints
		// This data source contains number 1 to 4 which can be helpful for
		// programs to make smart decisions.
		this->hints.setDataAt(typeid(PrimitiveType<int>), 0, 1);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 1, 2);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 2, 3);
		this->hints.setDataAt(typeid(PrimitiveType<int>), 3, 4);
	};

	/// Destructor
	~StickGameWithOpponent() {};

	/// Inherited via LearningEnvironment
	virtual void doAction(uint64_t actionID) override;

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<DataHandlers::DataHandler>> getDataSources() override;

	/**
	* Inherited from LearningEnvironment.
	*
	* In this game, the score is 0 during the Game. At the end of the game
	* the score is 1 if the agent won, 0 if the agent lost, and -1 if the agent
	* took more sticks than available at the end of the game.
	*/
	virtual double getScore() const override;

	/// Inherited via LearningEnvironment
	virtual bool isTerminal() const override;
};

#endif