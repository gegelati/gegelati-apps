#ifndef CVT_MAP_ELITES_ARCHIVE_H
#define CVT_MAP_ELITES_ARCHIVE_H

#include <gegelati.h>
#include "mapElitesArchive.h"

class CvtMapElitesArchive : public MapElitesArchive {
    protected:
    
        std::vector<std::vector<double>> centroids;

        size_t nbCentroids;             
        size_t nbDotsInit = 10000;              
        size_t nbIterationInit = 200; 
        
        
        const double a1 = 0.5, a2 = 0.5;
        const double b1 = 0.5, b2 = 0.5;

    public:

        CvtMapElitesArchive(std::vector<double>& archiveLimits, size_t dim, uint64_t size, Mutator::RNG& rng)
            : MapElitesArchive(archiveLimits, dim), nbCentroids{size}
        {
            centroids.resize(nbCentroids);
            archive.resize(size);
            initialize_cvt(rng);
        }

        double dist_squared(const std::vector<double>& a, const std::vector<double>& b);
        std::vector<double> add(const std::vector<double>& a, const std::vector<double>& b);
        std::vector<double> scalar_mult(const std::vector<double>& a, double s);
        std::vector<double> average(const std::vector<std::vector<double>>& points);
        std::vector<double> random_point(Mutator::RNG& rng);
        size_t nearest(const std::vector<double>& point, const std::vector<std::vector<double>>& centroids);

        void initialize_cvt(Mutator::RNG& rng);

        uint64_t size() const;

        std::pair<uint64_t, uint64_t> getDimensions() const;


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