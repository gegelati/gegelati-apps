
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <string>

#include <gegelati.h>

namespace Selector {

    namespace MapElites {

        namespace CustomDescriptors {

            /**
             * Class for FeetContact descriptor.
             * 
             * \brief This descriptor records which feet are in contact with the ground
             * during the evaluation of an agent. Each foot's contact status is represented
             * as a binary value (1 for contact, 0 for no contact) over the duration of the evaluation.
             */
            class FeetContact : public MapElitesDescriptor
            {
              public:
                /**
                 * \brief Specialisation of initDescriptor
                 */
                virtual void initDescriptor(const TPG::TPGGraph& graph,
                                            const Learn::LearningEnvironment&
                                                learningEnvironment) override;

                      
                /**
                 * \brief Specialisation of getName
                 */                          
                virtual std::string getName() const override;

                /**
                 * \brief Specialisation of extractMetricsStep
                 */
                virtual void extractMetricsStep(
                    std::vector<double>& metrics, const TPG::TPGVertex* agent,
                    std::vector<double> actionValues,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;

                /**
                 * \brief Specialisation of extractMetricsEpisode
                 */
                virtual void extractMetricsEpisode(
                    std::vector<double>& metrics, const TPG::TPGVertex* agent,
                    size_t nbStepsExecuted,
                    const Learn::LearningEnvironment& learningEnvironment)
                    const override;
            };

        }; // namespace DefaultDescriptors
    }; // namespace MapElites
}; // namespace Selector


/*
namespace Descriptor{

    enum class DescriptorType {
        FeetContact,
        ActionValues,
        NbInstr,
        NbInstrUseful,
        NbUniqueInstr,
        NbUniqueInstrUseful
    };

    inline std::string descriptorTypeToString(DescriptorType type) {
        switch (type) {
            case DescriptorType::FeetContact: return "FeetContact";
            case DescriptorType::ActionValues: return "ActionValues";
            case DescriptorType::NbInstr: return "NbInstr";
            case DescriptorType::NbInstrUseful: return "NbInstrUseful";
            case DescriptorType::NbUniqueInstr: return "NbUniqueInstr";
            case DescriptorType::NbUniqueInstrUseful: return "NbUniqueInstrUseful";
            default: throw std::runtime_error("Type not known.");
        }
    }

    inline DescriptorType StringToDescriptorType(const std::string& typeStr) {
        if (typeStr == "FeetContact") {
            return DescriptorType::FeetContact;
        } else if (typeStr == "ActionValues") {
            return DescriptorType::ActionValues;
        } else if (typeStr == "NbInstr") {
            return DescriptorType::NbInstr;
        } else if (typeStr == "NbInstrUseful") {
            return DescriptorType::NbInstrUseful;
        } else if (typeStr == "NbUniqueInstr") {
            return DescriptorType::NbUniqueInstr;
        } else if (typeStr == "NbUniqueInstrUseful") {
            return DescriptorType::NbUniqueInstrUseful;
        } else {
            throw std::runtime_error("Type not known.");
        }
    }

    inline bool isBehavioralDescriptor(DescriptorType type){
        switch (type) {
            case DescriptorType::FeetContact: return true;
            case DescriptorType::ActionValues: return true;

            case DescriptorType::NbInstr: return false;
            case DescriptorType::NbInstrUseful: return false;
            case DescriptorType::NbUniqueInstr: return false;
            case DescriptorType::NbUniqueInstrUseful: return false;
            default: throw std::runtime_error("Type not known.");
        }
    }
    
    inline bool isGenotypicDescriptor(DescriptorType type){
        return !isBehavioralDescriptor(type);
    }
};
*/

#endif