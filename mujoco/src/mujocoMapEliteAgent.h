
#ifndef MUJOCO_MAP_ELITE_AGENT_H
#define MUJOCO_MAP_ELITE_AGENT_H

#include <gegelati.h>
#include "mujocoEnvironment/mujocoWrapper.h"
#include "mapElitesArchive.h"
#include "cvtMapElitesArchive.h"

namespace Learn {

    
    class MujocoMapEliteLearningAgent : public ParallelLearningAgent 
    {
        protected:

            MapElitesArchive* mapEliteArchive = nullptr;
            const ArchiveParametrization& archiveParams;

            bool useOnlyCloseAddEdges;
   

        public: 
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const ArchiveParametrization& archiveParams,
                const LearningParameters& p, bool useOnlyCloseAddEdges = false,

                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), archiveParams{archiveParams},
                useOnlyCloseAddEdges{useOnlyCloseAddEdges} {

                }

            const ArchiveParametrization& getArchiveParams() {return archiveParams;}

            virtual void init(uint64_t seed = 0) override;
            
            virtual void trainOneGeneration(uint64_t generationNumber) override;

            virtual void decimateWorstRoots(
                std::multimap<std::shared_ptr<EvaluationResult>,
                            const TPG::TPGVertex*>& results) override;


            std::vector<double> updateDescriptorWithMainValues(std::vector<double>& descriptors);

            virtual std::shared_ptr<EvaluationResult> evaluateJob(
                TPG::TPGExecutionEngine& tee, const Job& job,
                uint64_t generationNumber, LearningMode mode,
                LearningEnvironment& le) const override;

            virtual void updateEvaluationRecords(
                const std::multimap<std::shared_ptr<EvaluationResult>,
                                    const TPG::TPGVertex*>& results) override;

            virtual const MapElitesArchive& getMapElitesArchive();

    };
}

# endif