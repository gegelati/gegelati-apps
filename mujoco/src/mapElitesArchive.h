

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
            if(dim1 > 0 && dim2 > 0){
                archive.resize(std::pow(dim1, dim2));
            }
        }

        uint64_t size() const;

        std::pair<uint64_t, uint64_t> getDimensions() const;

        virtual const std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>& getAllArchive() const;

        virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
            getArchiveAt(const std::vector<uint64_t>& indices) const;



        virtual const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(
            const std::vector<double>& descriptors) const; 

        virtual void setArchiveAt(
            const TPG::TPGVertex* vertex,
            std::shared_ptr<Learn::EvaluationResult> eval,
            const std::vector<uint64_t>& indices);
    
        virtual void setArchiveFromDescriptors(
            const TPG::TPGVertex* vertex, 
            std::shared_ptr<Learn::EvaluationResult> eval, 
            const std::vector<double>& descriptors);  

        virtual uint64_t getIndexArchive(double value) const;
        virtual uint64_t computeLinearIndex(const std::vector<uint64_t>& indices) const;


        virtual void initCSVarchive(std::string path) const ;
        virtual void updateCSVArchive(std::string path, uint64_t generationNumber) const;

        virtual bool containsRoot(const TPG::TPGVertex* root) const;

        virtual void removeRootFromArchiveIfNotComplete(const TPG::TPGVertex* root, size_t maxNbEvaluation);
        virtual void removeRootFromArchive(const TPG::TPGVertex* root, size_t maxNbEvaluation);


    };
    

#endif