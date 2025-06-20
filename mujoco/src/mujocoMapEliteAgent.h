
#ifndef MUJOCO_MAP_ELITE_AGENT_H
#define MUJOCO_MAP_ELITE_AGENT_H

#include <gegelati.h>
#include "mujocoEnvironment/mujocoWrapper.h"
#include "mapElitesArchive.h"

namespace Learn {
    
    class MujocoMapEliteLearningAgent : public ParallelLearningAgent 
    {
        protected:

            MapEliteArchive mapEliteArchive;

        public: 
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const LearningParameters& p, std::vector<double> archiveLimits={},
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), mapEliteArchive(archiveLimits, 4)
            {

                if(mapEliteArchive.size() > 0 && p.maxNbEvaluationPerPolicy != p.nbIterationsPerPolicyEvaluation){
                    throw std::runtime_error("With map elites, an agent should not be evaluated again");
                }
            }

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

            virtual const MapEliteArchive& getMapElitesArchive();

    };
}

# endif