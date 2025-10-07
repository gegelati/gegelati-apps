#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

#include "mujocoWrapper.h"


std::vector<std::reference_wrapper<const Data::DataHandler>> MujocoWrapper::getDataSources()
{
	auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
	result.push_back(this->currentState);
	return result;
}

void MujocoWrapper::reset(size_t seed, Learn::LearningMode mode, uint16_t iterationNumber, uint64_t generationNumber)
{
	// Create a seed from the input seed and mode
	size_t hash_seed = Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);
	if(mode == Learn::LearningMode::VALIDATION){
		hash_seed = 6416846135168433+iterationNumber;
	}

	saveStateAndAction = mode == Learn::LearningMode::TESTING;

	// Reset the RNG and environment state
	this->rng.setSeed(hash_seed);
	this->nbActionsExecuted = 0;
	this->totalReward = 0.0;
	this->totalUtility = 0.0;

	this->obstacleIndex = 0;
	this->obstaclePos = 0;
	this->currentObstacleArea = 0;

	this->deactivateAllObstacles();
}

void MujocoWrapper::initialize_simulation() {

	// Load and compile model
	char error[1000] = "Could not load binary model";
	m_ = mj_loadXML(model_path_.c_str(), 0, error, 1000);
	if (!m_) {
		char formattedError[256];
		sprintf(formattedError, "Load model error: %s", error);
		mju_error(formattedError);
	}
	// Make data
	d_ = mj_makeData(m_);

	std::copy_n(d_->qpos, m_->nq, back_inserter(init_qpos_));
	std::copy_n(d_->qvel, m_->nv, back_inserter(init_qvel_));
}

void MujocoWrapper::set_state(std::vector<double>& qpos, std::vector<double>& qvel) {
	// Set the joints position qpos and velocity qvel of the model.
	// Note: `qpos` and `qvel` are not the full physics state for all mujoco
	// models/environments
	// https://mujoco.readthedocs.io/en/stable/APIreference/APItypes.html#mjtstate

	for (int i = 0; i < m_->nq; i++) d_->qpos[i] = qpos[i];
	for (int i = 0; i < m_->nv; i++) d_->qvel[i] = qvel[i];
	mj_forward(m_, d_);
}


void MujocoWrapper::do_simulation(std::vector<double>& ctrl, int n_frames) {
	for (int i = 0; i < m_->nu && i < static_cast<int>(ctrl.size()); i++) {
		d_->ctrl[i] = ctrl[i];
	}
	for (int i = 0; i < n_frames; i++) {
		mj_step(m_, d_);
	}
	// As of MuJoCo 2.0, force - related quantities like cacc are not
	// computed unless there's a force sensor in the model. See https:
	// // github.com/openai/gym/issues/1541
	mj_rnePostConstraint(m_, d_);
}


Data::PrimitiveTypeArray<double>& MujocoWrapper::getCurrentState()
{
	return currentState;
}


std::string MujocoWrapper::ExpandEnvVars(const std::string &str) {
	std::string result;
	size_t pos = 0;

	while (pos < str.length()) {
		if (str[pos] == '$') {
			size_t start = pos + 1;
			size_t end = start;

			// Handle ${VAR} format
			if (start < str.length() && str[start] == '{') {
				end = str.find('}', start);
				if (end != std::string::npos) {
					std::string varName =
						str.substr(start + 1, end - start - 1);
					const char *varValue = getenv(varName.c_str());
					if (varValue) {
						result += varValue;
					}
					pos = end + 1;
					continue;
				}
			}

			// Handle $VAR format
			while (end < str.length() &&
				(isalnum(str[end]) || str[end] == '_')) {
				++end;
			}
			std::string varName = str.substr(start, end - start);
			const char *varValue = getenv(varName.c_str());
			if (varValue) {
				result += varValue;
			}
			pos = end;
		} else {
			result += str[pos];
			++pos;
		}
	}
	return result;
}

void MujocoWrapper::registerStateAndAction(const std::vector<double>& actionsID)
{
	if(saveStateAndAction){
		actionData.push_back(actionsID);

		std::vector<double> state;
		for(size_t i = 0; i < stateSize; i++){
			state.push_back(*this->currentState.getDataAt(typeid(double), i).getSharedPointer<const double>());
		}
		stateData.push_back(state);
	}
}

void MujocoWrapper::printStateAndAction(std::string path) const
{
	if(saveStateAndAction){

		std::ofstream outFile(path);
		if (!outFile.is_open()) {
			std::cerr << "Archive file could not be created " << path << std::endl;
			return;
		} else if (actionData.size() == 0 ||stateData.size() == 0 || actionData.size() != stateData.size()){
			throw std::runtime_error("Action or state data is empty, or don't have the same size");
		}
		size_t nbActionData = actionData[0].size();
		size_t nbStateData = stateData[0].size();

		// Write CSV header
		outFile << "step,";
		for(size_t i = 0; i < nbActionData; i++){
			outFile << "action"<<i<<",";
		}
		for(size_t i = 0; i < nbStateData; i++){
			outFile << "state"<<i<<",";
		}
		outFile<<"\n";

		for(size_t step = 0; step < actionData.size(); step++){
			outFile<<step<<",";

			for(size_t i = 0; i < nbActionData; i++){
				outFile << actionData[step][i]<<",";
			}
			for(size_t i = 0; i < nbStateData; i++){
				outFile << stateData[step][i]<<",";
			}
			outFile<<"\n";
		}
		outFile.close();
	} else {
		throw std::runtime_error("saveStateAndAction should be true");
	}

}


bool MujocoWrapper::computeObstaclesState(uint64_t index, double xposMin, double xposMax, bool resetCall)
{
	double dist_x = 0.0;
	bool obstacleSucced = false;

	// If no possible obstacle

	// Move to the next area?
	if (resetCall || d_->qpos[0] > sizeObstacleArea * (currentObstacleArea + 1)) {
		if (d_->qpos[0] > sizeObstacleArea * (currentObstacleArea + 1)) {
			currentObstacleArea++;
			obstacleSucced = true;
		}

		if (activeObstacles.empty()) {
			// Set ground 0 for no obstacle
			this->activateCurrentObstacle(0, 0, (sizeObstacleArea * currentObstacleArea) + sizeObstacleArea / 2);
			currentState.setDataAt(typeid(double), index, -1); // obstacleIndex has no meaning here
			currentState.setDataAt(typeid(double), index + 1, 0.0);
			return false;
		} else {
			// Randomly select an obstacle from the list
			size_t idx = this->rng.getUnsignedInt64(0, activeObstacles.size() - 1);
			this->obstacleIndex = activeObstacles[idx];
			this->activateCurrentObstacle(
				(sizeObstacleArea * currentObstacleArea) + xposMin,
				(sizeObstacleArea * (currentObstacleArea + 1)) - xposMax,
				(sizeObstacleArea * currentObstacleArea) + sizeObstacleArea / 2);
			dist_x = obstaclePos - d_->qpos[0];
		}
	}
	else if (!activeObstacles.empty()) {
		dist_x = obstaclePos - d_->qpos[0];
	}
	currentState.setDataAt(typeid(double), index, this->obstacleIndex);
	currentState.setDataAt(typeid(double), index + 1, dist_x);

	return obstacleSucced;
}

// Move all obstacles and grounds to -100 (out of play)
void MujocoWrapper::deactivateAllObstacles() {
	for (const auto& [idx, name] : allObstacles) {
		int id = mj_name2id(m_, mjOBJ_BODY, name.c_str());
		if (id != -1) m_->body_pos[3 * id] = -100.0;
	}
	for (const auto& [idx, name] : allGrounds) {
		int id = mj_name2id(m_, mjOBJ_BODY, name.c_str());
		if (id != -1) m_->body_pos[3 * id] = -100.0;
	}
}

// Active the current obstacle in the sequence, deactivate the others
void MujocoWrapper::activateCurrentObstacle(double xposMin, double xposMax, double middle) {
	this->deactivateAllObstacles();
	if(activeObstacles.size() == 0) {
		std::string groundName = "obstacle0-ground";
		int ground_id = mj_name2id(m_, mjOBJ_BODY, groundName.c_str());
		if (ground_id != -1) m_->body_pos[3 * ground_id] = middle;
	} else {
		double xpos = this->rng.getDouble(xposMin, xposMax) + additionObstacle;
		obstaclePos = xpos - additionObstacle;

		std::string obsName = "obstacle" + std::to_string(obstacleIndex);
		int obs_id = mj_name2id(m_, mjOBJ_BODY, obsName.c_str());
		if (obs_id != -1) m_->body_pos[3 * obs_id] = xpos;

		std::string groundName = obsName + "-ground";
		int ground_id = mj_name2id(m_, mjOBJ_BODY, groundName.c_str());
		if (ground_id != -1) m_->body_pos[3 * ground_id] = middle;
	}
}

// Prepare the sequence of obstacles to use (in the given order)
void MujocoWrapper::setObstacles(const std::vector<size_t>& obs) {
	activeObstacles = obs;
	this->deactivateAllObstacles();
	noObstacleActive = obs.empty();
	// obstacleIndex is not modified here, it will be updated during selection
}