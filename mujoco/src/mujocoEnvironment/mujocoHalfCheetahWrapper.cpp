#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "mujocoHalfCheetahWrapper.h"




void MujocoHalfCheetahWrapper::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber)
{
	MujocoWrapper::reset(seed, mode, iterationNumber, generationNumber);



	std::vector<double> qpos(m_->nq);
	for (size_t i = 0; i < 9; i++) {
		qpos[i] = init_qpos_[i] + this->rng.getDouble(-reset_noise_scale_, reset_noise_scale_);
	}
	std::vector<double> qvel(m_->nv);
	for (size_t i = 0; i < 9; i++) {
		qvel[i] = init_qvel_[i] + this->rng.getDouble(0.0, reset_noise_scale_);
	}
	mj_resetData(m_, d_);
	set_state(qpos, qvel);

	

	
	this->updateObstaclesPosition(obstacleIndex, 0, 0);
	
	this->computeState();
	this->computeObstaclesState(17, 2, 2);
}

void MujocoHalfCheetahWrapper::doActions(std::vector<double> actionsID)
{
    this->registerStateAndAction(actionsID);

	auto x_pos_before = d_->qpos[0];
	do_simulation(actionsID, frame_skip_);
	auto x_pos_after = d_->qpos[0];
	auto x_vel = (x_pos_after - x_pos_before) / (m_->opt.timestep * frame_skip_);


	auto forward_reward = forward_reward_weight * x_vel;
	auto rewards = forward_reward;
	
	auto costs = control_cost(actionsID);

	auto reward = rewards - costs;

	this->computeState();
	this->computeObstaclesState(17, 2, 2);

	// Incremente the reward.
	this->totalReward += rewards;
	if(!useObstacleReward){
		this->totalReward -= costs;
	}
	this->totalUtility += reward;

	this->nbActionsExecuted++;
}

bool MujocoHalfCheetahWrapper::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* MujocoHalfCheetahWrapper::clone() const
{
	return new MujocoHalfCheetahWrapper(*this);
}

double MujocoHalfCheetahWrapper::getScore() const
{
	return this->totalReward;
}
double MujocoHalfCheetahWrapper::getUtility() const
{
	return totalUtility;
}
bool MujocoHalfCheetahWrapper::isUsingUtility() const
{
	return useObstacleReward;
}


bool MujocoHalfCheetahWrapper::isTerminal() const
{
	return !is_healthy();
}

bool MujocoHalfCheetahWrapper::is_healthy() const{
	return (d_->qpos[2] < 2.5) && (d_->qpos[2] > -2.5);
}

double MujocoHalfCheetahWrapper::control_cost(std::vector<double>& action) {
	double cost = 0;
	for (auto& a : action) cost += a * a;
	return control_cost_weight_ * cost;
}


void MujocoHalfCheetahWrapper::computeState(){
	int index = 0;
	int reduction = 0;
	if(exclude_current_positions_from_observation_){
		reduction = 1;
	}
	for (int i = reduction; i < 9; i++) 
	{
		currentState.setDataAt(typeid(double), index, d_->qpos[i]);
		index++;
	}
	for (int i = 0; i < 9; i++) 
	{
		currentState.setDataAt(typeid(double), index, d_->qvel[i]);
		index++;
	}

}

