#ifndef MUJOCO_ANT_WRAPPER_H
#define MUJOCO_ANT_WRAPPER_H

#include <gegelati.h>
#include "mujocoWrapper.h"

class MujocoAntWrapper : public MujocoWrapper
{
protected:

	const std::string xmlFile;
	bool use_healthy_reward;
    bool exclude_current_positions_from_observation_;
    std::vector<double> healthy_z_range_;
    // Parameters
    double control_cost_weight_ = 0.5;
    double healthy_reward_ = 1.0;
	double forward_reward_weight = 1.0;
    bool terminate_when_unhealthy_ = true;
    double reset_noise_scale_ = 0.1;
	uint64_t main_body = 1;

public:

	/**
	* \brief Default constructor.
	*
	* Attributes angle and velocity are set to 0.0 by default.
	*/
	MujocoAntWrapper(const char *pXmlFile, std::vector<Descriptor::DescriptorType> descriptorTypes={}, bool useHealthyReward=true, bool p_exclude_current_positions_from_observation = true) :
		MujocoWrapper(8, (p_exclude_current_positions_from_observation) ? 27:29, descriptorTypes), 
		xmlFile{pXmlFile}, use_healthy_reward{useHealthyReward},
		exclude_current_positions_from_observation_{p_exclude_current_positions_from_observation}
		{
			model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
			healthy_z_range_ = {0.2, 1.0};
			initialize_simulation();

			// Initialize the descriptors
			initialize_descriptors();

		};

    /**
    * \brief Copy constructor for the armLearnWrapper.
    */ 
    MujocoAntWrapper(const MujocoAntWrapper &other) : MujocoWrapper(other), 
	xmlFile{other.xmlFile}, use_healthy_reward{other.use_healthy_reward},
	exclude_current_positions_from_observation_{other.exclude_current_positions_from_observation_}
	
	{
		model_path_ = MujocoWrapper::ExpandEnvVars(other.xmlFile);
		healthy_z_range_ = {0.2, 1.0};
		initialize_simulation();
		// Initialize the descriptors
		initialize_descriptors();
    }

    ~MujocoAntWrapper() {
        // Free visualization storage
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
	* \brief Get a score for the pendulum stabilization.
	*
	* The score returned at any time can either be positive or negative.
	*
	* A positive score is returned if the pendulum has been stabilized, that is,
	* the isTerminal() method returns true.
	* In such a case, the returned score will be $10 / ln(nbActionExecuted)$
	* such that shorter convergence time leads to higher scores.
	*
	* A negative score is returned if the pendulum has not been stabilized
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
	* \brief Is the pendulum considered stabilized.
	*
	* If the mean reward over the recent rewardHistory is lower than a fixed
	* threshold, then the pendulum is considered to be stable in the upward
	* position and the learningAgent has succeded in stabilizing it.
	*
	* \return true if the pendulum has been stabilized, false otherwise.
	*/
	virtual bool isTerminal() const override;


    double healthy_reward();

    double control_cost(std::vector<double>& action);

	void computeState();

    bool is_healthy() const;


	virtual void initialize_descriptors() override;
};

#endif // !MUJOCOANTWRAPPER_H