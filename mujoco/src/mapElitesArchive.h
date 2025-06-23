

#ifndef MAP_ELITES_ARCHIVE_H
#define MAP_ELITES_ARCHIVE_H

#include <gegelati.h>

class MapElitesArchive {
    protected:

    
        std::vector<double> archiveLimits;
        uint64_t dim1;
        uint64_t dim2;
        std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>> archive;

    public:

        MapElitesArchive(std::vector<double>& archiveLimits, uint64_t dim2)
            : archiveLimits{archiveLimits}, dim1{archiveLimits.size()}, dim2{dim2}
        {
            archive.resize(std::pow(dim1, dim2));
        }

        uint64_t size() const;

        std::pair<uint64_t, uint64_t> getDimensions() const;

        const std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>& getAllArchive() const;

        const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
            getArchiveAt(const std::vector<uint64_t>& indices) const;



        const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(
            const std::vector<double>& descriptors) const; 

        void setArchiveAt(
            const TPG::TPGVertex* vertex,
            std::shared_ptr<Learn::EvaluationResult> eval,
            const std::vector<uint64_t>& indices);
    
        void setArchiveFromDescriptors(
            const TPG::TPGVertex* vertex, 
            std::shared_ptr<Learn::EvaluationResult> eval, 
            const std::vector<double>& descriptors);  

        uint64_t getIndexArchive(double value) const;
        uint64_t computeLinearIndex(const std::vector<uint64_t>& indices) const;

        virtual void initCSVarchive(std::string path) const ;
        virtual void updateCSVArchive(std::string path, uint64_t generationNumber) const;

};


#endif