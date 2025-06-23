#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "mujocoAntWrapper.h"




void MujocoAntWrapper::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber)
{
	// Create seed from seed and mode
	size_t hash_seed = Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);
	if(mode == Learn::LearningMode::VALIDATION){
		hash_seed = 6416846135168433+iterationNumber;
	}

	// Reset the RNG
	this->rng.setSeed(hash_seed);


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
	this->nbActionsExecuted = 0;
	this->totalReward = 0.0;
	this->totalUtility = 0.0;


	// Reset feet in contact
	nb_feet_in_contact_ = {0.0, 0.0, 0.0, 0.0};
}

void MujocoAntWrapper::doActions(std::vector<double> actionsID)
{
	auto x_pos_before = d_->qpos[0];

	do_simulation(actionsID, frame_skip_);
	auto x_pos_after = d_->qpos[0];
	auto x_vel = (x_pos_after - x_pos_before) / (m_->opt.timestep * frame_skip_);
	auto forward_reward = x_vel * forward_reward_weight;
	auto rewards = forward_reward;
	
	auto ctrl_cost = control_cost(actionsID);
	auto costs = ctrl_cost;
	auto reward = rewards - costs;

	this->computeState();

	double healthyReward = healthy_reward();

	// Incremente the reward.
	this->totalReward += reward + int(use_healthy_reward) * healthyReward;
	this->totalUtility += reward + healthyReward;

	this->nbActionsExecuted++;

	if(useFeetContact){
		computeFeetContact();
	}

}

bool MujocoAntWrapper::isCopyable() const
{
	return true;
}

bool MujocoAntWrapper::isUsingUtility() const
{
	return !use_healthy_reward;
}

Learn::LearningEnvironment* MujocoAntWrapper::clone() const
{
	return new MujocoAntWrapper(*this);
}

double MujocoAntWrapper::getScore() const
{
	return totalReward;
}
double MujocoAntWrapper::getUtility() const
{
	return totalUtility;
}

bool MujocoAntWrapper::isTerminal() const
{
	return (terminate_when_unhealthy_ && !is_healthy());
}

double MujocoAntWrapper::healthy_reward() {
	return static_cast<double>(is_healthy()) * healthy_reward_;
}

double MujocoAntWrapper::control_cost(std::vector<double>& action) {
	double cost = 0;
	for (auto& a : action) cost += a * a;
	return control_cost_weight_ * cost;
}


bool MujocoAntWrapper::is_healthy() const{
	for (int i = 0; i < m_->nq; i++)
		if (!std::isfinite(d_->qpos[i])) return false;
	for (int i = 0; i < m_->nv; i++)
		if (!std::isfinite(d_->qvel[i])) return false;
	return (d_->qpos[2] >= healthy_z_range_[0] &&
			d_->qpos[2] <= healthy_z_range_[1]);
}


void MujocoAntWrapper::computeState(){
	int index = 0;
	int reduction = 0;
	if(exclude_current_positions_from_observation_){
		reduction = 2;
	}
	for (int i = reduction; i < m_->nq; i++) 
	{
		currentState.setDataAt(typeid(double), index, d_->qpos[i]);
		index++;
	}
	for (int i = 0; i < m_->nv; i++) 
	{
		currentState.setDataAt(typeid(double), index, d_->qvel[i]);
		index++;
	}
}

void MujocoAntWrapper::computeFeetContact() {
    
	std::set<uint64_t> increasedLegs;

    for (int i = 0; i < d_->ncon; ++i) {
        const mjContact& contact = d_->contact[i];
        int geom1 = contact.geom1;
        int geom2 = contact.geom2;

        // Check if geom1 is a foot
        auto it1 = footGeomToIndex.find(geom1);
        if (it1 != footGeomToIndex.end() && increasedLegs.find(it1->second) == increasedLegs.end()) {
            nb_feet_in_contact_[it1->second]++;
            increasedLegs.insert(it1->second);
            continue;
        }

        // Check if geom2 is a foot
        auto it2 = footGeomToIndex.find(geom2);
        if (it2 != footGeomToIndex.end() && increasedLegs.find(it2->second) == increasedLegs.end()) {
            nb_feet_in_contact_[it2->second]++;
            increasedLegs.insert(it2->second);
        }
    }
}

