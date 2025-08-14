
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
            std::vector<double> archiveLimits;

            bool usePonderationSelection;
            bool useOnlyCloseAddEdges;
            bool useCVT;
            size_t sizeCVT;
            bool useMeanDescriptor;
            bool useMedianDescriptor;
            bool useAbsMeanDescriptor;
            bool useQuantileDescriptor;
            bool useMinMaxDescriptor;
            size_t nbDescriptorsAnalysis;

        public: 
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const LearningParameters& p, std::vector<double> archiveLimits={},
                bool usePonderationSelection = false, bool useOnlyCloseAddEdges = false,
                bool useCVT = false, size_t sizeCVT = 100,
                bool useMeanDescriptor = false, bool useMedianDescriptor = false,
                bool useAbsMeanDescriptor = false, bool useQuantileDescriptor = false,
                bool useMinMaxDescriptor = false,
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), archiveLimits{archiveLimits},
                usePonderationSelection{usePonderationSelection}, 
                useOnlyCloseAddEdges{useOnlyCloseAddEdges}, useCVT{useCVT}, sizeCVT{sizeCVT},
                useMeanDescriptor{useMeanDescriptor}, useMedianDescriptor{useMedianDescriptor},
                useAbsMeanDescriptor{useAbsMeanDescriptor}, useQuantileDescriptor{useQuantileDescriptor},
                useMinMaxDescriptor{useMinMaxDescriptor} {
                    // At least one descriptor type must be used
                    if(!useMeanDescriptor && !useMedianDescriptor &&
                       !useAbsMeanDescriptor && !useQuantileDescriptor && !useMinMaxDescriptor){
                        throw std::runtime_error("At least one descriptor type must be used.");
                    }

                    // absMean can only be used alone
                    if(useAbsMeanDescriptor && (useMeanDescriptor || useMedianDescriptor ||
                        useQuantileDescriptor || useMinMaxDescriptor)){
                        std::cout<<" Warning: useAbsMeanDescriptor is set to true with other ones, since it range between 0, and 1 it will be less effective"<<std::endl;
                    }
                }

            virtual void init(uint64_t seed = 0) override;
            
            virtual void trainOneGeneration(uint64_t generationNumber) override;

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

            virtual const MapElitesArchive& getMapElitesArchive();

    };
}

# endif