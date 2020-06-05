#ifndef TIC_TAC_TOE_WITH_OPPONENT_H
#define TIC_TAC_TOE_WITH_OPPONENT_H

#include <random>

#include <gegelati.h>


/**
* LearningEnvironment to play the tic tac toe game against a random player.
* The principle of the tic tac toe is as follows. we have a 3x3 board.
* Alternatively, each player plays a turn, and has to put a circle (for a player) or a cross (for the other) in a cell.
* The player succeeding to align 3 of his symbols in a row, column or in diagonal wins
*
* In this LearningEnvironment, the trained agent plays against a random algo
*/
class TicTacToe : public Learn::LearningEnvironment {
protected:
    /// Current board containing -1 as empty, 0 as circles and 1 as crosses, size is 3*3 (row-order) (AI is circle)
    Data::PrimitiveTypeArray<int> board;

    /// The current turn being played
    int currentTurn;

    /// Did the player win or lose at the end of a game.
    bool win;

    /// Did the player attempt a forbidden move (i.e. put a cross in a cell where there is already a cross)
    bool forbiddenMove;

    /// Is the game null
    bool null;

    /// Is the game finished
    bool end;

    /// Randomness control
    Mutator::RNG rng;

    /// Utility function to quickly read a cell of the board
    virtual int getSymbolAt(int location) const;

    /// Updates the current winning state of the game : is it null ? Has smbdy won ?
    virtual void updateGame();

    /// Randomly plays on an empty cell for the given player
    virtual void randomPlay(int symbolOfPlayer);

public:
    /**
    * Constructor.
    */
    TicTacToe() : LearningEnvironment(9), board(9) {
        this->reset(0);
    };

    /// Destructor
    ~TicTacToe() {};

    /// Inherited via LearningEnvironment
    virtual void doAction(uint64_t actionID) override;

    /// Inherited via LearningEnvironment
    virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

    /// Inherited via LearningEnvironment
    virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;

    /**
    * Inherited from LearningEnvironment.
    *
    * In this game, the score is 0 during the Game. At the end of the game
    * the score is 1 if the agent won, 0 if it is null, -1 if the agent lost, and a malus is given if there was
    * a forbidden move
    */
    virtual double getScore() const override;

    /// Inherited via LearningEnvironment
    virtual bool isTerminal() const override;

};

#endif