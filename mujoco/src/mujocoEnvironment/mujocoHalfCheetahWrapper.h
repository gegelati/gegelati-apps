#ifndef MUJOCO_HALF_CHEETAH_WRAPPER_H
#define MUJOCO_HALF_CHEETAH_WRAPPER_H

#include <gegelati.h>
#include "mujocoWrapper.h"

class MujocoHalfCheetahWrapper : public MujocoWrapper
{
protected:

	/// Path to the MuJoCo XML model file
	const std::string xmlFile;
	/// Whether to use obstacle reward in the utility calculation
	bool useObstacleReward = false;

    // Parameters
    double forward_reward_weight = 1.0;
    double control_cost_weight_ = 0.1;
    double reset_noise_scale_ = 0.1;
    bool exclude_current_positions_from_observation_ = true;

public:

	MujocoHalfCheetahWrapper(const char *pXmlFile, bool useObstacleReward=true, std::vector<size_t> obstacleUsed={}, bool exclude_current_positions_from_observation = true) :
		MujocoWrapper(6, (exclude_current_positions_from_observation) ? 19:18), xmlFile{pXmlFile},
		useObstacleReward{useObstacleReward},
		exclude_current_positions_from_observation_{exclude_current_positions_from_observation}
		{
			model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
			initialize_simulation();

			sizeObstacleArea = 10;
		};

    /**
    * \brief Copy constructor for the MujocoHalfCheetahWrapper.
    */ 
    MujocoHalfCheetahWrapper(const MujocoHalfCheetahWrapper &other) : MujocoWrapper(other),
	xmlFile{other.xmlFile}, useObstacleReward{other.useObstacleReward},
	exclude_current_positions_from_observation_{other.exclude_current_positions_from_observation_}
	{   
		model_path_ = MujocoWrapper::ExpandEnvVars(other.xmlFile);
		initialize_simulation();
	}

    ~MujocoHalfCheetahWrapper() {
        // Free visualization storage (commented out, see platform notes)
        //mjv_freeScene(&scn_);
        //mjr_freeContext(&con_);

        // Free MuJoCo model and data
        mj_deleteData(d_);
        mj_deleteModel(m_);

        // Terminate GLFW (crashes with Linux NVidia drivers)
#if defined(__APPLE__) || defined(_WIN32)
        glfwTerminate();
#endif
    }

	/// Inherited via LearningEnvironment
	virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING,
					   uint16_t iterationNumber = 0, uint64_t generationNumber = 0) override;

	/// Inherited via LearningEnvironment
	virtual void doActions(std::vector<double> actionsID) override;

	/// Inherited via LearningEnvironment
	virtual bool isCopyable() const override;

	/// Inherited via LearningEnvironment
	virtual LearningEnvironment* clone() const;

	/**
	* \brief Get a score for the agent.
	*
	* The score returned at any time can either be positive or negative.
	*
	* A positive score is returned if the agent has succeeded, that is,
	* the isTerminal() method returns true.
	* In such a case, the returned score will be $10 / ln(nbActionExecuted)$
	* such that shorter convergence time leads to higher scores.
	*
	* A negative score is returned if the agent has not succeeded
	* (yet).
	* In such a case, the returned score simply is the average reward since
	* the last reset.
	*
	* \return a double value corresponding to the score.
	*/
	virtual double getScore() const override;
	virtual double getUtility() const override;

	/// Inherited via LearningEnvironment
	virtual bool isUsingUtility() const override;

	/**
	* \brief Is the agent considered successful (healthy).
	*
	* If the mean reward over the recent rewardHistory is lower than a fixed
	* threshold, then the agent is considered to be stable and has succeeded.
	*
	* \return true if the agent is healthy, false otherwise.
	*/
	virtual bool isTerminal() const override;
    bool is_healthy() const;

    /// Compute the control cost for a given action
    double control_cost(std::vector<double>& action);

	/// Compute and update the current state
	void computeState();

};

#endif // !MUJOCO_HALF_CHEETAH_WRAPPER_H