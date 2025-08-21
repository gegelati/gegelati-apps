#ifndef MUJOCO_WALKER2D_WRAPPER_H
#define MUJOCO_WALKER2D_WRAPPER_H

#include <gegelati.h>
#include "mujocoWrapper.h"

class MujocoWalker2DWrapper : public MujocoWrapper
{
protected:


	const std::string xmlFile;
	bool useHealthyReward;
	bool exclude_current_positions_from_observation_ = true;
	// Parameters
	double forward_reward_weight = 1.0;
	double control_cost_weight_ = 1e-3;
	double healthy_reward_ = 1.0;
	bool terminate_when_unhealthy_ = true;
	std::vector<double> healthy_z_range_;
	std::vector<double> healthy_angle_range_;
	double reset_noise_scale_ = 5e-3;

public:

	




	/**
	* \brief Default constructor.
	*
	* Attributes angle and velocity are set to 0.0 by default.
	*/
	MujocoWalker2DWrapper(const char *pXmlFile, std::string descriptorType = "unused", bool useHealthyReward_p=true, bool exclude_current_positions_from_observation = true) :
		MujocoWrapper(6, (exclude_current_positions_from_observation) ? 17:18, descriptorType), 
		xmlFile{pXmlFile}, useHealthyReward{useHealthyReward_p},
		exclude_current_positions_from_observation_{exclude_current_positions_from_observation}
		{
			model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
			healthy_z_range_ = {0.8, 2};
			healthy_angle_range_ = {-1.0, 1.0};
			frame_skip_ = 4;
			initialize_simulation();

			
			// Initialize the descriptors
			initialize_descriptors();
		};

    /**
    * \brief Copy constructor for the armLearnWrapper.
    */ 
    MujocoWalker2DWrapper(const MujocoWalker2DWrapper &other) : MujocoWrapper(other), 
	xmlFile{other.xmlFile}, useHealthyReward{other.useHealthyReward},
	exclude_current_positions_from_observation_{other.exclude_current_positions_from_observation_}

	{
		model_path_ = MujocoWrapper::ExpandEnvVars(other.xmlFile);
		healthy_z_range_ = {0.8, 2};
		healthy_angle_range_ = {-1.0, 1.0};
		frame_skip_ = 4;
		initialize_simulation();
		
		// Initialize the descriptors
		initialize_descriptors();
    }

    ~MujocoWalker2DWrapper() {
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

	void computeState();

    double control_cost(std::vector<double>& action);

    bool is_healthy() const;


	
	virtual void initialize_descriptors() override;

	virtual const size_t getNbDescriptors() override;
};

#endif // !MUJOCO_WALKER2D_WRAPPER_H