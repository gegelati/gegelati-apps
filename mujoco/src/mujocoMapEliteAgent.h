
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
            std::string descriptorName;
            bool useMeanDescriptor;
            bool useMedianDescriptor;
            bool useAbsMeanDescriptor;
            bool useQuantileDescriptor;
            bool useMinMaxDescriptor;
            size_t nbDescriptorsAnalysis;

            size_t nbMainDescriptors = 0;
            bool useMainMeanDescriptor;
            bool useMainMedianDescriptor;
            bool useMainStdDescriptor;
            bool useMainMaxDescriptor;
            bool useMainMinDescriptor;

            std::string typeProgramDescriptor;

        public: 
            MujocoMapEliteLearningAgent(
                MujocoWrapper& le, const Instructions::Set& iSet,
                const LearningParameters& p, std::vector<double> archiveLimits={},
                bool usePonderationSelection = false, bool useOnlyCloseAddEdges = false,
                bool useCVT = false, size_t sizeCVT = 100, std::string descriptorName = "feetContact",
                bool useMeanDescriptor = false, bool useMedianDescriptor = false,
                bool useAbsMeanDescriptor = true, bool useQuantileDescriptor = false,
                bool useMinMaxDescriptor = false, bool useMainMeanDescriptor = false,
                bool useMainMedianDescriptor = false, bool useMainStdDescriptor = false,
                bool useMainMaxDescriptor = false, bool useMainMinDescriptor = false,
                std::string typeProgramDescriptor = "None",
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory), archiveLimits{archiveLimits},
                usePonderationSelection{usePonderationSelection}, 
                useOnlyCloseAddEdges{useOnlyCloseAddEdges}, useCVT{useCVT}, sizeCVT{sizeCVT}, descriptorName{descriptorName},
                useMeanDescriptor{useMeanDescriptor}, useMedianDescriptor{useMedianDescriptor},
                useAbsMeanDescriptor{useAbsMeanDescriptor}, useQuantileDescriptor{useQuantileDescriptor},
                useMinMaxDescriptor{useMinMaxDescriptor}, useMainMeanDescriptor{useMainMeanDescriptor},
                useMainMedianDescriptor{useMainMedianDescriptor}, useMainStdDescriptor{useMainStdDescriptor},
                useMainMaxDescriptor{useMainMaxDescriptor}, useMainMinDescriptor{useMainMinDescriptor},
                typeProgramDescriptor{typeProgramDescriptor} {
                    // At least one descriptor type must be used
                    if(!useMeanDescriptor && !useMedianDescriptor &&
                       !useAbsMeanDescriptor && !useQuantileDescriptor && !useMinMaxDescriptor){
                        throw std::runtime_error("At least one descriptor type must be used.");
                    }

                    // absMean can only be used alone
                    if(useAbsMeanDescriptor && (useMeanDescriptor || useMedianDescriptor ||
                        useQuantileDescriptor || useMinMaxDescriptor)){
                        std::cerr<<" Warning: useAbsMeanDescriptor is set to true with other ones, since it range between 0, and 1 it will be less effective"<<std::endl;
                    }

                    nbMainDescriptors = useMainMeanDescriptor + useMainMedianDescriptor +
                        useMainStdDescriptor + useMainMaxDescriptor + useMainMinDescriptor;
                }

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