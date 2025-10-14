#ifndef CVT_MAP_ELITES_ARCHIVE_H
#define CVT_MAP_ELITES_ARCHIVE_H

#include <gegelati.h>
#include "mapElitesArchive.h"

class CVTArchiveParametrization : public ArchiveParametrization{
    public:

        size_t nbCentroids;   
        double minRange;
        double maxRange;

        size_t nbDotsInit;              
        size_t nbIterationInit; 

        double a1; 
        double a2;
        double b1;
        double b2;

        CVTArchiveParametrization(
            size_t nbDescriptors, std::vector<double> archiveLimits = {}, double minRange = 0.0, double maxRange = 1.0,
            size_t nbCentroids = 100, std::string descriptorName = "feetContact",
            bool useMeanDescriptor = false, bool useMedianDescriptor = false,
            bool useAbsMeanDescriptor = true, bool useQuantileDescriptor = false,
            bool useMinMaxDescriptor = false, bool useMainMeanDescriptor = false,
            bool useMainMedianDescriptor = false, bool useMainStdDescriptor = false,
            bool useMainMaxDescriptor = false, bool useMainMinDescriptor = false,
            std::string typeProgramDescriptor = "None",
            size_t nbDotsInit = 1000, size_t nbIterationInit = 300, double a1 = 0.5, double a2 = 0.5, double b1 = 0.5, double b2 = 0.5
        ): ArchiveParametrization(nbDescriptors, archiveLimits, descriptorName, useMeanDescriptor, 
                                  useMedianDescriptor, useAbsMeanDescriptor, useQuantileDescriptor, 
                                  useMinMaxDescriptor, useMainMeanDescriptor, useMainMedianDescriptor, 
                                  useMainStdDescriptor, useMainMaxDescriptor, useMainMinDescriptor, 
                                  typeProgramDescriptor), nbCentroids{nbCentroids}, minRange{minRange}, maxRange{maxRange}, nbDotsInit{nbDotsInit}, 
                                  nbIterationInit{nbIterationInit}, a1{a1}, a2{a2}, b1{b1}, b2{b2} 
        {
            if(!useAbsMeanDescriptor && descriptorName == "actionValues" && minRange == 0.0){
                minRange = maxRange * -1;
            }
        }

};

class CvtMapElitesArchive : public MapElitesArchive {
    protected:
    
        const CVTArchiveParametrization& archiveParams;
        std::vector<std::vector<double>> centroids;

    public:

        CvtMapElitesArchive(const CVTArchiveParametrization& archiveParams, Mutator::RNG& rng)
            : MapElitesArchive(archiveParams), archiveParams{archiveParams}
        {
            centroids.resize(archiveParams.nbCentroids);
            archive.resize(archiveParams.nbCentroids);
            initialize_cvt(rng);
        }

        double dist_squared(const std::vector<double>& a, const std::vector<double>& b);
        std::vector<double> add(const std::vector<double>& a, const std::vector<double>& b);
        std::vector<double> scalar_mult(const std::vector<double>& a, double s);
        std::vector<double> average(const std::vector<std::vector<double>>& points);
        std::vector<double> random_point(Mutator::RNG& rng);
        size_t nearest(const std::vector<double>& point, const std::vector<std::vector<double>>& centroids);

        void initialize_cvt(Mutator::RNG& rng);


        size_t getIndexForDescriptor(const std::vector<double>& descriptors) const;

        const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(
            const std::vector<double>& descriptors) const override; 

    
        void setArchiveFromDescriptors(
            const TPG::TPGVertex* vertex, 
            std::shared_ptr<Learn::EvaluationResult> eval, 
            const std::vector<double>& descriptors) override;  



        virtual void initCSVarchive(std::string path) const override;
        virtual void updateCSVArchive(std::string path, uint64_t generationNumber) const override;

};


#endif