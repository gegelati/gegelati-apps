#ifndef CVT_MAP_ELITES_ARCHIVE_H
#define CVT_MAP_ELITES_ARCHIVE_H

#include <gegelati.h>
#include <mapElitesArchive.h>

class CvtMapElitesArchive : public MapElitesArchive {
    protected:
    
        uint64_t size;
        std::vector<double> centroids;

    public:

        CvtMapElitesArchive(std::vector<double>& archiveLimits, uint64_t size)
            : MapElitesArchive(archiveLimits, 1), size{size}
        {
            archive.resize(size);
            initialize_cvt();
        }

        void initialize_cvt();

        uint64_t size() const;

        std::pair<uint64_t, uint64_t> getDimensions() const;

        const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
            getArchiveAt(const std::vector<uint64_t>& indices) const;



        const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& getArchiveFromDescriptors(
            const std::vector<double>& descriptors) const override; 

        void setArchiveAt(
            const TPG::TPGVertex* vertex,
            std::shared_ptr<Learn::EvaluationResult> eval,
            const std::vector<uint64_t>& indices);
    
        void setArchiveFromDescriptors(
            const TPG::TPGVertex* vertex, 
            std::shared_ptr<Learn::EvaluationResult> eval, 
            const std::vector<double>& descriptors) override;  

        uint64_t getIndexArchive(double value) const;
        uint64_t computeLinearIndex(const std::vector<uint64_t>& indices) const;


        virtual void initCSVarchive(std::string path) const override;
        virtual void updateCSVArchive(std::string path, uint64_t generationNumber) const override;

};


#endif