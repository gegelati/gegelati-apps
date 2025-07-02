#ifndef MUJOCO_DOUBLE_PENDULUM_WRAPPER_H
#define MUJOCO_DOUBLE_PENDULUM_WRAPPER_H

#include <gegelati.h>
#include "mujocoWrapper.h"

class MujocoDoublePendulumWrapper : public MujocoWrapper
{
protected:

	/// Randomness control
	Mutator::RNG rng;

	/// Total reward accumulated since the last reset
	double totalReward = 0.0;
    double totalUtility = 0.0;

	/// Number of actions since the last reset
	uint64_t nbActionsExecuted = 0;


	const std::string xmlFile;
	// Parameters
	double reset_noise_scale_ = 5e-3;
public:

	




	/**
	* \brief Default constructor.
	*
	* Attributes angle and velocity are set to 0.0 by default.
	*/
	MujocoDoublePendulumWrapper(const char *pXmlFile, std::string descriptorType = "unused") :
		MujocoWrapper(1, 11, descriptorType), 
		xmlFile{pXmlFile}
		{
			model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
			initialize_simulation();
			if(descriptorType_ == DescriptorType::FeetContact){
				throw std::runtime_error("Descriptor type FeetContact is not supported for MujocoDoublePendulumWrapper.");
			} else if(descriptorType_ == DescriptorType::ActionValues){
				// Initialize the descriptors
				initialize_descriptors();
			}
			
		};

    /**
    * \brief Copy constructor for the armLearnWrapper.
    */ 
    MujocoDoublePendulumWrapper(const MujocoDoublePendulumWrapper &other) : MujocoWrapper(other),
	xmlFile{other.xmlFile}
	{
		model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
		initialize_simulation();
		if(descriptorType_ == DescriptorType::FeetContact){
			throw std::runtime_error("Descriptor type FeetContact is not supported for MujocoDoublePendulumWrapper.");
		} else if(descriptorType_ == DescriptorType::ActionValues){
			// Initialize the descriptors
			initialize_descriptors();
		}
    }

    ~MujocoDoublePendulumWrapper() {
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



	void computeState();


};

#endif // !MUJOCO_DOUBLE_PENDULUM_WRAPPER_H