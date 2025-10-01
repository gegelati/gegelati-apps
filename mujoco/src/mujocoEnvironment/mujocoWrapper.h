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

	
	std::multimap<size_t, std::string> allObstacles = {
		{0, "obstacle0"}, {1, "obstacle1"}, {2, "obstacle2"}, {3, "obstacle3"}, {4, "obstacle4"}
	};
	std::multimap<size_t, std::string> allGrounds = {
		{0, "obstacle0-ground"}, {1, "obstacle1-ground"}, {2, "obstacle2-ground"}, {3, "obstacle3-ground"}, {4, "obstacle4-ground"}
	};


	std::multimap<size_t, std::string> obstacles;
	std::multimap<size_t, std::string> grounds;
	int64_t obstacleIndex = 0;
	uint64_t currentObstacleArea = 0;
	double sizeObstacleArea = 0.0;
	double obstaclePos = 0.0;
	double additionObstacle = 0.0;
	bool noObstacleArea = false;

public:

	/**
	* \brief Default constructor.
	*
	* Attributes angle and velocity are set to 0.0 by default.
	*/
	MujocoWrapper(uint64_t nbActions, uint64_t stateSize) :
		LearningEnvironment(nbActions, false),
		currentState{ stateSize }, stateSize{stateSize}
	{};

	/**
	* \brief Copy constructor for the MujocoWrapper.
	*
	* Default copy constructor since all attributes are trivially copyable.
	*/
	MujocoWrapper(const MujocoWrapper& other) : LearningEnvironment(other.nbActions, false),
		currentState{other.currentState}, stateSize{other.stateSize}, sizeObstacleArea{other.sizeObstacleArea}, additionObstacle{other.additionObstacle} {}


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

    void initialize_simulation();

    void set_state(std::vector<double>& qpos, std::vector<double>& qvel);

    void do_simulation(std::vector<double>& ctrl, int n_frames);


	Data::PrimitiveTypeArray<double>& getCurrentState();
	uint64_t getStateSize(){
		return stateSize;
	}

	std::string ExpandEnvVars(const std::string &str);


	virtual void registerStateAndAction(const std::vector<double>& actionsID);
	virtual void printStateAndAction(std::string path) const;

	virtual void updateObstaclesPosition(int64_t indexObstacle, double xposMin, double xposMax, double middle);

	virtual bool computeObstaclesState(uint64_t index, double xposMin, double xposMax);

	virtual void setObstacles(std::vector<size_t>& obs);

};

#endif // !MUJOCO_WRAPPER_H