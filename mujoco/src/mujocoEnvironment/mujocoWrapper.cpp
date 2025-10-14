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
	// Create seed from seed and mode
	size_t hash_seed = Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);
	if(mode == Learn::LearningMode::VALIDATION || mode == Learn::LearningMode::TESTING){
		hash_seed = 6416846135168433+iterationNumber;
	}

	saveStateAndAction = mode == Learn::LearningMode::TESTING;

	// Reset the RNG
	this->rng.setSeed(hash_seed);
	this->nbActionsExecuted = 0;
	this->totalReward = 0.0;
	this->totalUtility = 0.0;

	// Reset descriptors
	if(descriptorType_ != DescriptorType::Unused){
		descriptors.clear();
	}
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
	// Note: `qpos` and `qvel` is not the full physics state for all mujoco
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

const size_t MujocoWrapper::getNbDescriptors()
{
	return nbActions;
}


void MujocoWrapper::computeDescriptors(std::vector<double>& actionsID) {
	if(descriptorType_ == DescriptorType::FeetContact){
		//std::cout<<descriptors.size()<<" Compute."<<std::endl;
		computeFeetContact();
	} else if (descriptorType_ == DescriptorType::ActionValues) {
		std::vector<double> descriptorValues;
		for(size_t i = 0; i < actionsID.size(); ++i) {
			descriptorValues.push_back(actionsID[i]);
		}
		descriptors.push_back(descriptorValues);
	}
}

void MujocoWrapper::computeFeetContact() {
    
	std::vector<double> descriptorsValues(feet_geom_ids_.size(), 0.0);
	std::set<uint64_t> increasedLegs;

    for (int i = 0; i < d_->ncon; ++i) {
        const mjContact& contact = d_->contact[i];
        int geom1 = contact.geom1;
        int geom2 = contact.geom2;

        // Check if geom1 is a foot
        auto it1 = footGeomToIndex.find(geom1);
        if (it1 != footGeomToIndex.end() && increasedLegs.find(it1->second) == increasedLegs.end()) {
            descriptorsValues[it1->second]++;
            increasedLegs.insert(it1->second);
            continue;
        }

        // Check if geom2 is a foot
        auto it2 = footGeomToIndex.find(geom2);
        if (it2 != footGeomToIndex.end() && increasedLegs.find(it2->second) == increasedLegs.end()) {
            descriptorsValues[it2->second]++;
            increasedLegs.insert(it2->second);
        }
    }

	descriptors.push_back(descriptorsValues);
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

		// Header
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