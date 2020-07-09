#include "TicTacToe.h"

double TicTacToe::getSymbolAt(int location) const {
    return (double) *((this->board.getDataAt(typeid(double), location)).getSharedPointer<const double>());
}

void TicTacToe::play(uint64_t actionID, double symbolOfPlayer) {
    if (!this->isTerminal()) {
        double cellContent = this->getSymbolAt((int)actionID);
        if (cellContent != -1.0) {
            std::cout << "Non-empty cell ! Random play is being done" << std::endl;
            this->randomPlay(symbolOfPlayer);
        } else {
            this->board.setDataAt(typeid(double), actionID, symbolOfPlayer);
        }
        this->currentTurn++;
        updateGame();
        if (this->isTerminal()) {
            std::cout << "End of game !" << std::endl;
            if(null){
                std::cout << "Null game" << std::endl;
                return;
            }
            if(winPlayer1){
                std::cout << "Circle won, well done !" << std::endl;
                return;
            }
            std::cout << "Cross won, well done !" << std::endl;
        }
    }
}

void TicTacToe::doAction(uint64_t actionID) {
    LearningEnvironment::doAction(actionID);

    bool &forbiddenMove =
            (currentTurn % 2 == 0 ? forbiddenMovePlayer1 : forbiddenMovePlayer2);

    bool &win =
            (currentTurn % 2 == 0 ? forbiddenMovePlayer1 : forbiddenMovePlayer2);

    int symbOfPlayer = 1; // the board has been modified so that each player is circle

    // if the game is not over
    if (!this->isTerminal()) {
        // Execute the action

        double cellContent = this->getSymbolAt((int)actionID);
        // Checks the move is possible
        if (cellContent != -1.0) {
            // Illegal move : we play randomly
            forbiddenMove = true;
            this->randomPlay(symbOfPlayer);
        } else {
            // update state
            this->board.setDataAt(typeid(double), actionID, symbOfPlayer);
        }

        this->currentTurn++;
        // checking the state of the game to see if it is finished
        this->updateGame();

        this->revertBoard();
    }
}

void TicTacToe::revertBoard() {
    for (int i=0;i<board.getLargestAddressSpace(); i++){
        double cell = (double)*(this->board.getDataAt(typeid(double), i)).getSharedPointer<const double>();
        board.setDataAt(typeid(double),i,cell == 0 ? 1 : cell==1?0:-1);
    }
}

void TicTacToe::randomPlay(double symbolOfPlayer) {
    int nbEmptyCellsRemaining = 9 - this->currentTurn;
    int decision = (int)rng.getUnsignedInt64(0, nbEmptyCellsRemaining - 1);
    int i = -1;
    // we're now looking for the empty slot n°decision
    while (decision >= 0) {
        i++;
        // check if the cell is empty
        if (this->getSymbolAt(i) == -1) {
            decision--;
        }
    }

    // i is the position of the n°decision empty slot
    // update state
    this->board.setDataAt(typeid(double), i, symbolOfPlayer);
}

void TicTacToe::reset(size_t seed, Learn::LearningMode mode) {
    // Create seed from seed and mode
    size_t hash_seed = Data::Hash<size_t>()(seed) ^Data::Hash<Learn::LearningMode>()(mode);
    this->rng.setSeed(hash_seed);
    // sets -1 for each cell to make them empty
    for (int i = 0; i < 9; i++) {
        this->board.setDataAt(typeid(double), i, -1.0);
    }
    this->currentTurn = 0;
    this->winPlayer1 = false;
    this->winPlayer2 = false;
    this->forbiddenMovePlayer1 = false;
    this->forbiddenMovePlayer2 = false;
    this->null = false;
    this->end = false;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> TicTacToe::getDataSources() {
    auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
    result.push_back(this->board);
    return result;
}

std::shared_ptr<Learn::AdversarialEvaluationResult> TicTacToe::getScores() const {
    // adds a malus if there has been a forbiden move in the game
    double malusForbiddenMoveP1 = this->forbiddenMovePlayer1 ? 1.0 : 0.0;
    double malusForbiddenMoveP2 = this->forbiddenMovePlayer2 ? 1.0 : 0.0;
    // check if the game is null
    if (this->null) {
        return std::make_shared<Learn::AdversarialEvaluationResult>(
                Learn::AdversarialEvaluationResult(
                        {0.5-malusForbiddenMoveP1,0.5-malusForbiddenMoveP2}));
    }
    // check if the P1 won
    if (this->winPlayer1) {
        return std::make_shared<Learn::AdversarialEvaluationResult>(
                Learn::AdversarialEvaluationResult(
                        {1-malusForbiddenMoveP1,0-malusForbiddenMoveP2}));
    }

    // P1 lost
    return std::make_shared<Learn::AdversarialEvaluationResult>(
            Learn::AdversarialEvaluationResult(
                    {0-malusForbiddenMoveP1,1-malusForbiddenMoveP2}));
}

void TicTacToe::updateGame() {
    // we will check if there is a row/col/diag winning combination by looking at every possibility

    bool& win = (currentTurn%2==0 ? winPlayer1 : winPlayer2);

    double x00 = this->getSymbolAt(0);
    double x10 = this->getSymbolAt(1);
    double x20 = this->getSymbolAt(2);
    double x01 = this->getSymbolAt(3);
    double x11 = this->getSymbolAt(4);
    double x21 = this->getSymbolAt(5);
    double x02 = this->getSymbolAt(6);
    double x12 = this->getSymbolAt(7);
    double x22 = this->getSymbolAt(8);

    //printf("{%d,%d,%d\n%d,%d,%d\n%d,%d,%d}\n\n",x00,x10,x20,x01,x11,x21,x02,x12,x22);


    // looking for combinations containing x11
    if (x11 != -1 && (x01 == x11 && x11 == x21
                      || x10 == x11 && x11 == x12
                      || x00 == x11 && x11 == x22
                      || x20 == x11 && x11 == x02)) {
        this->end = true;
        win = true;
        return;
    }

    // looking for other combinations containing x22
    if (x22 != -1.0 && (x02 == x12 && x12 == x22
                        || x20 == x21 && x21 == x22)) {

        win = true;

        this->end = true;
        return;
    }


    // looking for other combinations containing x00
    if (x00 != -1.0 && (x00 == x10 && x10 == x20
                        || x00 == x01 && x01 == x02)) {

        win = true;
        this->end = true;
        return;
    }

    // nobody won, if already 9 turns happened we are stuck with a null game
    if (this->currentTurn > 8) {
        this->null = true;
        this->end = true;
    }
}

bool TicTacToe::isTerminal() const {
    return this->end;
}

bool TicTacToe::isCopyable() const {
    return true;
}

Learn::LearningEnvironment *TicTacToe::clone() const {
    return new TicTacToe(*this);
}

std::string TicTacToe::toString() const {
    std::stringstream res;
    res << "-------\n";
    res << "|" << cellToString(0) << "|" << cellToString(1) << "|" << cellToString(2) << "|\n";
    res << "-------\n";
    res << "|" << cellToString(3) << "|" << cellToString(4) << "|" << cellToString(5) << "|\n";
    res << "-------\n";
    res << "|" << cellToString(6) << "|" << cellToString(7) << "|" << cellToString(8) << "|\n";
    res << "-------\n";
    return res.str();
}

std::string TicTacToe::cellToString(int pos) const{
    double symbol = getSymbolAt(pos);
    return symbol == 0 ? "O" : symbol == 1 ? "X" : " ";
}