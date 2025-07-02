#ifndef MUJOCO_HUMANOID_WRAPPER_H
#define MUJOCO_HUMANOID_WRAPPER_H

#include <gegelati.h>
#include "mujocoWrapper.h"

class MujocoHumanoidWrapper : public MujocoWrapper
{
protected:
    Mutator::RNG rng;
    double totalReward = 0.0;
    double totalUtility = 0.0;
    uint64_t nbActionsExecuted = 0;
    const std::string xmlFile;

    bool use_healthy_reward;
    bool use_contact_forces_;
    bool exclude_current_positions_from_observation_;
    double forward_reward_weight_ = 1.25;
    double control_cost_weight_ = 0.1;
    double healthy_reward_ = 5.0;
    bool terminate_when_unhealthy_ = true;
    double contact_cost_weight_ = 5e-4;
    std::vector<double> healthy_z_range_ = {1.0, 2.0};
    std::vector<double> contact_force_range_ = {-1.0, 1.0};
    double reset_noise_scale_ = 1e-2;
public:

    // Constructeur
    MujocoHumanoidWrapper(const char *pXmlFile, std::string descriptorType = "unused", 
                          bool useHealthyReward = true, 
                          bool useContactForce = false,
                          bool excludeCurrentPositionsFromObservation = true) :
        MujocoWrapper(17, excludeCurrentPositionsFromObservation ? 376 : 378, descriptorType),
        xmlFile{pXmlFile},
        use_healthy_reward{useHealthyReward},
        use_contact_forces_{useContactForce},
        exclude_current_positions_from_observation_{excludeCurrentPositionsFromObservation}
    {
        model_path_ = MujocoWrapper::ExpandEnvVars(xmlFile);
        initialize_simulation();
        if(descriptorType_ == DescriptorType::FeetContact){
            throw std::runtime_error("Descriptor type FeetContact is not supported for MujocoHumanoidWrapper.");
        } else if(descriptorType_ == DescriptorType::ActionValues){
            // Initialize the descriptors
            initialize_descriptors();
        }
    }

    MujocoHumanoidWrapper(const MujocoHumanoidWrapper &other) :
        MujocoWrapper(other),
        xmlFile{other.xmlFile},
        use_healthy_reward{other.use_healthy_reward},
        use_contact_forces_{other.use_contact_forces_},
        exclude_current_positions_from_observation_{other.exclude_current_positions_from_observation_},
        forward_reward_weight_{other.forward_reward_weight_},
        control_cost_weight_{other.control_cost_weight_},
        healthy_reward_{other.healthy_reward_},
        terminate_when_unhealthy_{other.terminate_when_unhealthy_},
        healthy_z_range_{other.healthy_z_range_},
        reset_noise_scale_{other.reset_noise_scale_}
    {
        model_path_ = MujocoWrapper::ExpandEnvVars(other.xmlFile);
        initialize_simulation();
        if(descriptorType_ == DescriptorType::FeetContact){
            throw std::runtime_error("Descriptor type FeetContact is not supported for MujocoHumanoidWrapper.");
        } else if(descriptorType_ == DescriptorType::ActionValues){
            // Initialize the descriptors
            initialize_descriptors();
		}
    }


    ~MujocoHumanoidWrapper() {
        mj_deleteData(d_);
        mj_deleteModel(m_);
#if defined(__APPLE__) || defined(_WIN32)
        glfwTerminate();
#endif
    }

    virtual void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING, uint16_t iterationNumber = 0, uint64_t generationNumber = 0) override;
    virtual void doActions(std::vector<double> actionsID) override;
    virtual bool isCopyable() const override;
    virtual LearningEnvironment* clone() const override;
    virtual double getScore() const override;
	virtual double getUtility() const override;

	/// Inherited via LearningEnvironment
	virtual bool isUsingUtility() const override;
    virtual bool isTerminal() const override;

    double healthy_reward();
    double control_cost(std::vector<double>& action);
    std::vector<double> contact_forces();
    double contact_cost();
    bool is_healthy() const;
    std::array<double, 2> massCenter() const;
    virtual void computeState();
};

#endif // MUJOCO_HUMANOID_WRAPPER_H
