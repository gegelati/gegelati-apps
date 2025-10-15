#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>
#include <algorithm>

#include "mujocoHopperWrapper.h"




void MujocoHopperWrapper::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber)
{
	MujocoWrapper::reset(seed, mode, iterationNumber, generationNumber);


	std::vector<double> qpos(m_->nq);
	for (size_t i = 0; i < qpos.size(); i++) {
		qpos[i] = init_qpos_[i] + this->rng.getDouble(-reset_noise_scale_, reset_noise_scale_);
	}
	std::vector<double> qvel(m_->nv);
	for (size_t i = 0; i < qvel.size(); i++) {
		qvel[i] = init_qvel_[i] + this->rng.getDouble(0.0, reset_noise_scale_);
	}
	mj_resetData(m_, d_);
	set_state(qpos, qvel);
	this->computeState();
}

void MujocoHopperWrapper::doActions(std::vector<double> actionsID)
{
    this->registerStateAndAction(actionsID);

	auto x_pos_before = d_->qpos[0];
	do_simulation(actionsID, frame_skip_);
	auto x_pos_after = d_->qpos[0];
	auto x_vel = (x_pos_after - x_pos_before) / (m_->opt.timestep * frame_skip_);
	auto forward_reward = forward_reward_weight * x_vel;
	auto rewards = forward_reward;

	auto ctrl_cost = control_cost(actionsID);
	auto costs = ctrl_cost;

	auto reward = rewards - costs;

	this->computeState();

	// Incremente the reward.
	this->totalReward += reward + int(useHealthyReward) * healthy_reward();
	this->totalUtility += reward + healthy_reward();

	this->nbActionsExecuted++;


	computeDescriptors(actionsID);
}

bool MujocoHopperWrapper::isCopyable() const
{
	return true;
}

Learn::LearningEnvironment* MujocoHopperWrapper::clone() const
{
	return new MujocoHopperWrapper(*this);
}

double MujocoHopperWrapper::getScore() const
{
	return totalReward;
}
double MujocoHopperWrapper::getUtility() const
{
	return totalUtility;
}
bool MujocoHopperWrapper::isUsingUtility() const
{
	return !useHealthyReward;
}

bool MujocoHopperWrapper::isTerminal() const
{
	return (terminate_when_unhealthy_ && !is_healthy());
}

double MujocoHopperWrapper::healthy_reward() {
	return static_cast<double>(is_healthy() || terminate_when_unhealthy_) *
			healthy_reward_;
}

double MujocoHopperWrapper::control_cost(std::vector<double>& action) {
	double cost = 0;
	for (auto& a : action) cost += a * a;
	return control_cost_weight_ * cost;
}

bool MujocoHopperWrapper::is_healthy() const{
      double z = d_->qpos[1];
      double angle = d_->qpos[2];

      bool healthy_state = true;
	  size_t size_state = (exclude_current_positions_from_observation_) ? 11:12;
      for (size_t idx = 0; idx < size_state; idx++) {

		double s = *currentState.getDataAt(typeid(double), idx).getSharedPointer<const double>().get();
         if (!(healthy_state_range_[0] < s && s < healthy_state_range_[1])) {
            healthy_state = false;
            break;
         }
      }

      bool healthy_z = healthy_z_range_[0] < z && z < healthy_z_range_[1];
      bool healthy_angle =
          healthy_angle_range_[0] < angle && angle < healthy_angle_range_[1];
      return healthy_state && healthy_z && healthy_angle;
   }


void MujocoHopperWrapper::computeState(){
	int index = 0;
	int reduction = 0;
	if(exclude_current_positions_from_observation_){
		reduction = 1;
	}

	for (int i = reduction; i < m_->nq; i++) 
	{
		currentState.setDataAt(typeid(double), index, d_->qpos[i]);
		index++;
	}
	for (int i = 0; i < m_->nv; i++) 
	{
		currentState.setDataAt(typeid(double), index, std::clamp((double)d_->qvel[i], -10.0, 10.0));
		index++;
	}
}


void MujocoHopperWrapper::initialize_descriptors() {

	// Initialize values for feet contact
	feet_geom_ids_.clear();
	feet_geom_ids_.push_back(mj_name2id(m_, mjOBJ_GEOM, "foot_geom"));

	for (size_t j = 0; j < feet_geom_ids_.size(); ++j) {
		footGeomToIndex[feet_geom_ids_[j]] = j;
	}
	
	MujocoWrapper::initialize_descriptors();
	
}