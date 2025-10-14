


#ifndef MULTI_LEARNING_AGENT_H
#define MULTI_LEARNING_AGENT_H

#include "gegelati.h"
#include "lexicaseSelector.h"
#include "hierachicalSelector.h"
#include "multiEvaluationResults.h"

namespace Learn {
     class MultiLearningAgent : public ParallelLearningAgent
    {
        protected:
            std::vector<std::vector<size_t>> trainingTestCases;
            std::vector<std::vector<size_t>> validationTestCases;
            std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *> lastValidationResults;


        public:
            MultiLearningAgent(
                LearningEnvironment& le, const Instructions::Set& iSet,
                const LearningParameters& p, 
                         const std::vector<std::vector<size_t>>& trainingTestCases,
                         const std::vector<std::vector<size_t>>& validationTestCases = {}, const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), trainingTestCases{trainingTestCases}, validationTestCases{validationTestCases}
            {
                // There is probably a cleaner way to do that, but using the factory
                // was creating import issues.
                
                if(validationTestCases.size() == 0){
                    this->validationTestCases = trainingTestCases;
                }

                if (p.selection._isHierarchical){
                    if(params.mutation.tpg.ratioTeamsOverActions == 0 || params.mutation.tpg.ratioTeamsOverActions == 1){
                        throw std::runtime_error("ratio should be double between 0 and 1");
                    } else {
                        actionArchives.clear();
                        for(size_t idx = 0; idx < trainingTestCases.size(); idx++){
                            actionArchives.push_back(new Archive(p.archiveSize, p.archivingProbability));
                        }
                    }
                    

                    selector = std::make_shared<Selector::HierarchicalSelector>(tpg, p, trainingTestCases);
                } else if  (p.selection._selectionMode == "truncation") {
                    selector =
                        std::make_shared<Selector::TruncationSelector>(tpg, p);
                }
                else if (p.selection._selectionMode == "tournament") {
                    selector =
                        std::make_shared<Selector::TournamentSelector>(tpg, p);
                }
                else if (p.selection._selectionMode == "lexicase"){
                    selector = std::make_shared<Selector::LexicaseSelector>(tpg, p, trainingTestCases);
                } else {
                    throw std::runtime_error("Selection mode not found");
                }

            }

            virtual std::shared_ptr<EvaluationResult> evaluateJob(
                TPG::TPGExecutionEngine& tee, const Job& job,
                uint64_t generationNumber, LearningMode mode,
                LearningEnvironment& le) const override;


            virtual void trainOneGeneration(uint64_t generationNumber,
                                            bool doPopulate = true) override;

            virtual void init(uint64_t seed) override; 

            const std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *>& getLastValidationResults() const {
                return lastValidationResults;
            }

    };
}

#endif