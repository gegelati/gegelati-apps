
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


            /**
             * Class for NbInstr descriptor.
             * 
             * \brief This descriptor records the number of instructions for each program of the agent.
             * This descriptor is only usable for MAPLE.
             */
            class NbInstr : public MapElitesDescriptor
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

#endif