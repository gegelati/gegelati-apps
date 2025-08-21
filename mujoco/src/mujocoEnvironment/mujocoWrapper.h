#ifndef MUJOCO_WRAPPER_H
#define MUJOCO_WRAPPER_H

#include <gegelati.h>
#include <mujoco.h>

/**
* \brief Inverted pendulum LearningEnvironment.
*
* The code of the class is adapted from Florian Arrestier's code released
* under CECILL-C License.
* Link: https://github.com/preesm/preesm-apps/tree/master/org.ietr.preesm.reinforcement_learning
*/
class MujocoWrapper : public Learn::LearningEnvironment
{
protected:

	/// Randomness control
	Mutator::RNG rng;

	/// Total reward accumulated since the last reset
	double totalReward = 0.0;
    double totalUtility = 0.0;

	/// Number of actions since the last reset
	uint64_t nbActionsExecuted = 0;

	Data::PrimitiveTypeArray<double> currentState;
	uint64_t stateSize;

	std::vector<std::vector<double>> stateData;
	std::vector<std::vector<double>> actionData;
	bool saveStateAndAction = false;

	std::vector<int> feet_geom_ids_; // À initialiser avec les indices MuJoCo des pieds
	std::vector<bool> feet_in_contact_;
	std::unordered_map<int, size_t> footGeomToIndex;

public:

	enum class DescriptorType { FeetContact, ActionValues, Unused };

	/**
	* \brief Default constructor.
	*
	* Attributes angle and velocity are set to 0.0 by default.
	*/
	MujocoWrapper(uint64_t nbActions, uint64_t stateSize, std::string descriptorType="unused") :
		LearningEnvironment(nbActions, false),
		currentState{ stateSize }, stateSize{stateSize}
	{
		if(descriptorType == "feetContact"){
			descriptorType_ = DescriptorType::FeetContact;
		} else if (descriptorType == "actionValues"){
			descriptorType_ = DescriptorType::ActionValues;
		} else if (descriptorType == "unused" || descriptorType.empty()){
			descriptorType_ = DescriptorType::Unused;
		} else {
			throw std::runtime_error("Unknown descriptor type: " + descriptorType + ". Valid types are: feetContact and actionValues, 'unused' or nothing should be used to disable it .");
		}	
	};

	/**
	* \brief Copy constructor for the MujocoWrapper.
	*
	* Default copy constructor since all attributes are trivially copyable.
	*/
	MujocoWrapper(const MujocoWrapper& other) : LearningEnvironment(other.nbActions, false),
		currentState{other.currentState}, stateSize{other.stateSize}, 
		descriptorType_{other.descriptorType_} {}
	

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING,
					   uint16_t iterationNumber = 0, uint64_t generationNumber = 0) override;

	/// Inherited via LearningEnvironment
	virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;

	// MuJoCo data structures
    mjModel* m_ = NULL;  // MuJoCo model
    mjData* d_ = NULL;   // MuJoCo data
    mjvCamera cam_;      // abstract camera
    mjvOption opt_;      // visualization options
    mjvScene scn_;       // abstract scene
    mjrContext con_;     // custom GPU context

    std::vector<double> init_qpos_;  // Initial positions
    std::vector<double> init_qvel_;  // Initial velocities

    std::string model_path_;  // Absolute path to model xml file
    int frame_skip_ = 5;  // Number of frames per simlation step
    int obs_size_;  // Number of variables in observation vector

	std::vector<std::vector<double>> descriptors; // Descriptors
	DescriptorType descriptorType_ = DescriptorType::Unused;

    void initialize_simulation();

    void set_state(std::vector<double>& qpos, std::vector<double>& qvel);

    void do_simulation(std::vector<double>& ctrl, int n_frames);


	Data::PrimitiveTypeArray<double>& getCurrentState();
	uint64_t getStateSize(){
		return stateSize;
	}

	std::string ExpandEnvVars(const std::string &str);

	virtual void initialize_descriptors();
	virtual const size_t getNbDescriptors();
	virtual void computeDescriptors(std::vector<double>& actionsID);
	virtual const std::vector<std::vector<double>>& getDescriptors() const {
		return descriptors;
	}

	
	void computeFeetContact();

	virtual void registerStateAndAction(const std::vector<double>& actionsID);
	virtual void printStateAndAction(std::string path) const;

};

#endif // !MUJOCO_WRAPPER_H