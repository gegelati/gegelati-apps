#include"gridworld.h"

void GridWorld::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber){

    // Reset agent coordonate
    agentCoord = {0, 0};
    
    // Reset terminated and score
    terminated = false;
    score = 0.0;

    // Set data
    currentState.setDataAt(typeid(double), 0, agentCoord[0]);
    currentState.setDataAt(typeid(double), 1, agentCoord[1]);
}

bool GridWorld::positionAvailable(int pos_x, int pos_y){

    // position unavailable on axis x
    if(pos_x == size[0] || pos_x == -1){
        return false;
    }

    // position unavailable on axis x
    if(pos_y == size[1] || pos_y == -1){
        return false;
    }

    // position unavailable because tile is unavailable
    if (grid[pos_y][pos_x] == 3){
        return false;
    }

    // Else : position is available
    return true;

}

void GridWorld::doAction(double action){

    switch ((uint64_t)action){
        case 0: // left
            if (positionAvailable(agentCoord[0] - 1, agentCoord[1])) agentCoord[0]--;
            break;
        case 1: // Down
            if (positionAvailable(agentCoord[0], agentCoord[1] + 1)) agentCoord[1]++;
            break;
        case 2: // Right
            if (positionAvailable(agentCoord[0] + 1, agentCoord[1])) agentCoord[0]++;
            break;
        case 3: // Up
            if (positionAvailable(agentCoord[0], agentCoord[1] - 1)) agentCoord[1]--;
            break;
    }

    // Reward is always -1 except when an output is reached
    double reward = -1;

    if(grid[agentCoord[1]][agentCoord[0]] == 1){
        // good output reached
        terminated = true;
        reward = 100;
    } else if(grid[agentCoord[1]][agentCoord[0]] == 2){
        // Bad output reached
        terminated = true;
        reward = -100;
    }

    // update score
    score += reward;

    // Set data
    currentState.setDataAt(typeid(double), 0, agentCoord[0]);
    currentState.setDataAt(typeid(double), 1, agentCoord[1]);
}

bool GridWorld::isTerminal() const{
    return terminated;
}

double GridWorld::getScore() const {
    return score;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> GridWorld::getDataSources()
{
	auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
	result.push_back(this->currentState);
	return result;
}

Learn::LearningEnvironment* GridWorld::clone() const
{
	return new GridWorld(*this);
}

bool GridWorld::isCopyable() const
{
	return true;
}
