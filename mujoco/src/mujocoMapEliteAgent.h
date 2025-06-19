
#ifndef MUJOCO_MAP_ELITE_AGENT_H
#define MUJOCO_MAP_ELITE_AGENT_H

#include <gegelati.h>
#include "mujocoEnvironment/mujocoWrapper.h"

namespace Learn {
    
    class MujocoMapEliteLearningAgent : public ParallelLearningAgent 
    {
        protected:

        std::vector<double> archiveLimits;
        uint64_t sizeArchive;
        std::vector<std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>> archive;



        public: 
        MujocoMapEliteLearningAgent(
            MujocoWrapper& le, const Instructions::Set& iSet,
            const LearningParameters& p, std::vector<double> archiveLimits={},
            const TPG::TPGFactory& factory = TPG::TPGFactory())
            : ParallelLearningAgent(le, iSet, p, factory), archiveLimits{archiveLimits}, sizeArchive{archiveLimits.size()}
        {
            uint64_t nbFeets = 4;
            archive.resize(std::pow(sizeArchive, nbFeets));

            if(sizeArchive > 0 && p.maxNbEvaluationPerPolicy != p.nbIterationsPerPolicyEvaluation){
                throw std::runtime_error("With map elites, an agent should not be evaluated again");
            }
        }

        virtual void initCSVarchive(std::string path);
        virtual void updateCSVArchive(std::string path, size_t generationNumber);

        virtual void decimateWorstRoots(
            std::multimap<std::shared_ptr<EvaluationResult>,
                          const TPG::TPGVertex*>& results) override;


        virtual std::shared_ptr<EvaluationResult> evaluateJob(
            TPG::TPGExecutionEngine& tee, const Job& job,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const override;

        virtual void updateEvaluationRecords(
            const std::multimap<std::shared_ptr<EvaluationResult>,
                                const TPG::TPGVertex*>& results) override;

        const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& getArchiveAt(size_t i, size_t j, size_t k, size_t l);  
        const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(const std::vector<double>& descriptors); 

        void setArchiveAt(const TPG::TPGVertex* vertex, std::shared_ptr<EvaluationResult> eval, size_t i, size_t j, size_t k, size_t l);  
        void setArchiveFromDescriptors(const TPG::TPGVertex* vertex, std::shared_ptr<EvaluationResult> eval, const std::vector<double>& descriptors);  

        uint64_t getIndexArchive(double value);

    };
}

# endif