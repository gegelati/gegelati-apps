#ifndef TIC_TAC_TOE_WITH_OPPONENT_H
#define TIC_TAC_TOE_WITH_OPPONENT_H

#include <random>

#include <gegelati.h>

#include "TicTacToe.h"

/**
 * LearningEnvironment to play the tic tac toe game against a random player.
 * The principle of the tic tac toe is as follows. we have a 3x3 board.
 * Alternatively, each player plays a turn, and has to put a circle (for a
 * player) or a cross (for the other) in a cell. The player succeeding to align
 * 3 of his symbols in a row, column or in diagonal wins
 *
 * In this LearningEnvironment, the trained agent plays against a random algo
 */
class TicTacToe : public Learn::AdversarialLearningEnvironment {
protected:
  /// Current board containing -1 as empty, 0 as circles and 1 as crosses, size
  /// is 3*3 (row-order) (AI is circle)
  Data::PrimitiveTypeArray<double> board;

  /// The current turn being played
  int currentTurn;

  /// Did the player win or lose at the end of a game.
  bool winPlayer1;

  /// Did the player win or lose at the end of a game.
  bool winPlayer2;

  /// Did the player attempt a forbidden move (i.e. put a cross in a cell where
  /// there is already a cross)
  bool forbiddenMovePlayer1;

  /// Did the player attempt a forbidden move (i.e. put a cross in a cell where
  /// there is already a cross)
  bool forbiddenMovePlayer2;

  /// Is the game null
  bool null;

  /// Is the game finished
  bool end;

  /// Randomness control
  Mutator::RNG rng;

  /// In case the goal is not to do some adversarial learning, the opponent can
  /// be random
  bool isSecondPlayerRandom;

  /// Utility function to quickly read a cell of the board
  virtual double getSymbolAt(int location) const;

  /// Updates the current winning state of the game : is it null ? Has smbdy won
  /// ?
  virtual void updateGame();

  /// Randomly plays on an empty cell for the given player
  virtual void randomPlay(double symbolOfPlayer);

  // changes 0 to 1 and 1 to 0
  void revertBoard();

public:
  /**
   * Constructor.
   */
  TicTacToe(bool isSecondPlayerRandom = false)
      : AdversarialLearningEnvironment(9), board(9),
        isSecondPlayerRandom(isSecondPlayerRandom) {
    this->reset(0);
  };

  /**
   * \brief Copy constructor for the TicTacToe.
   *
   * Default copy constructor since all attributes are trivially copyable.
   */
  TicTacToe(const TicTacToe &other) = default;

  /// Destructor
  ~TicTacToe(){};

  /// Does a given move; used to manually test the game
  virtual void play(uint64_t actionID, double symbolOfPlayer);

  /// Inherited via LearningEnvironment
  virtual void doAction(double actionID) override;

  /// Inherited via LearningEnvironment
  virtual void
  reset(size_t seed = 0,
        Learn::LearningMode mode = Learn::LearningMode::TRAINING,
        uint16_t iterationNumber = 0,
        uint64_t generationNumber = 0) override;

  /// Inherited via LearningEnvironment
  virtual std::vector<std::reference_wrapper<const Data::DataHandler>>
  getDataSources() override;

  /**
   * Inherited from LearningEnvironment.
   *
   * In this game, the score is 0 during the Game. At the end of the game
   * the score is 1 if the agent won, 0.5 if it is null, 0 if the agent lost,
   * and a malus of -1 is given if there was a forbidden move. The
   * EvaluationResult contains the score of the 2 players.
   */
  virtual std::shared_ptr<Learn::AdversarialEvaluationResult>
  getScores() const override;

  /// Inherited via LearningEnvironment
  virtual bool isTerminal() const override;

  /// Inherited via LearningEnvironment
  virtual bool isCopyable() const override;

  /// Inherited via LearningEnvironment
  virtual LearningEnvironment *clone() const;

  /// Used to print the game
  virtual std::string toString() const;

  /// Used to help printing the board : translates int values to some characters
  /// (X,O, )
  virtual std::string cellToString(int pos) const;
};

#endif