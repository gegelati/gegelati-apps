
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
            std::map<Descriptor::DescriptorType, MapElitesArchive*> mapEliteArchives;
            const std::map<Descriptor::DescriptorType, const ArchiveParametrization*> archiveParams;            

            bool useOnlyCloseAddEdges;
   

        public: 
            // Multi archive
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const std::map<Descriptor::DescriptorType, const ArchiveParametrization*>& archiveParams,
                const LearningParameters& p, bool useOnlyCloseAddEdges = false,
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), archiveParams{archiveParams},
                useOnlyCloseAddEdges{useOnlyCloseAddEdges} {
                    if(archiveParams.size() == 0){
                        throw std::runtime_error("Need at least one archive");
                    }
                }

            // Single archive
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const ArchiveParametrization* archiveParams,
                const LearningParameters& p, bool useOnlyCloseAddEdges = false,
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), archiveParams{{archiveParams->descriptorType, archiveParams}},
                useOnlyCloseAddEdges{useOnlyCloseAddEdges} {}


            const ArchiveParametrization* getArchiveParamsAt(Descriptor::DescriptorType descriptor) {return archiveParams.at(descriptor);}

            virtual void init(uint64_t seed = 0) override;
            
            virtual void trainOneGeneration(uint64_t generationNumber) override;

            virtual void decimateWorstRoots(
                std::multimap<std::shared_ptr<EvaluationResult>,
                            const TPG::TPGVertex*>& results) override;


            std::vector<double> updateDescriptorWithMainValues(std::vector<double>& descriptors, Descriptor::DescriptorType descriptorType);

            virtual std::shared_ptr<EvaluationResult> evaluateJob(
                TPG::TPGExecutionEngine& tee, const Job& job,
                uint64_t generationNumber, LearningMode mode,
                LearningEnvironment& le) const override;

            virtual void updateEvaluationRecords(
                const std::multimap<std::shared_ptr<EvaluationResult>,
                                    const TPG::TPGVertex*>& results) override;

            virtual const MapElitesArchive& getMapElitesArchiveAt(Descriptor::DescriptorType descriptor);

            virtual void updateGenotypicDescriptors(
                std::vector<double>& descriptors, const ArchiveParametrization* localArchiveParam, 
                const TPG::TPGVertex* root) const;

            virtual void updateBehavioralDescriptors(
                std::vector<double>& descriptors, const ArchiveParametrization* localArchiveParam, 
                size_t nbDescriptors, const std::vector<std::vector<double>> environmentDescriptors) const;

    };
}

# endif