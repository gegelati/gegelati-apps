

#ifndef MAP_ELITES_ARCHIVE_H
#define MAP_ELITES_ARCHIVE_H

#include <gegelati.h>

class ArchiveParametrization
{
    public:
        virtual ~ArchiveParametrization() = default;
        size_t nbDescriptors;

        std::vector<double> archiveLimits;
        
        std::string descriptorName;
        
        bool useMeanDescriptor;
        bool useMedianDescriptor;
        bool useAbsMeanDescriptor;
        bool useQuantileDescriptor;
        bool useMinMaxDescriptor;


        bool useMainMeanDescriptor;
        bool useMainMedianDescriptor;
        bool useMainStdDescriptor;
        bool useMainMaxDescriptor;
        bool useMainMinDescriptor;

        std::string typeProgramDescriptor;

        size_t nbDescriptorsAnalysis;
        size_t nbMainDescriptors;

        ArchiveParametrization(
            size_t nbDescriptors, std::vector<double> archiveLimits = {},
            std::string descriptorName = "feetContact",
            bool useMeanDescriptor = false, bool useMedianDescriptor = false,
            bool useAbsMeanDescriptor = true, bool useQuantileDescriptor = false,
            bool useMinMaxDescriptor = false, bool useMainMeanDescriptor = false,
            bool useMainMedianDescriptor = false, bool useMainStdDescriptor = false,
            bool useMainMaxDescriptor = false, bool useMainMinDescriptor = false,
            std::string typeProgramDescriptor = "None"
        ): nbDescriptors{nbDescriptors}, archiveLimits{archiveLimits}, descriptorName{descriptorName},
            useMeanDescriptor{useMeanDescriptor}, useMedianDescriptor{useMedianDescriptor},
            useAbsMeanDescriptor{useAbsMeanDescriptor}, useQuantileDescriptor{useQuantileDescriptor},
            useMinMaxDescriptor{useMinMaxDescriptor}, useMainMeanDescriptor{useMainMeanDescriptor},
            useMainMedianDescriptor{useMainMedianDescriptor}, useMainStdDescriptor{useMainStdDescriptor},
            useMainMaxDescriptor{useMainMaxDescriptor}, useMainMinDescriptor{useMainMinDescriptor},
            typeProgramDescriptor{typeProgramDescriptor} {
            // Compute the number of descriptors analysed
            this->nbDescriptorsAnalysis = useMeanDescriptor + useMedianDescriptor +
                useAbsMeanDescriptor + useQuantileDescriptor * 2 + useMinMaxDescriptor * 2;

            // At least one descriptor type must be used
            if(!useMeanDescriptor && !useMedianDescriptor &&
                !useAbsMeanDescriptor && !useQuantileDescriptor && !useMinMaxDescriptor){
                throw std::runtime_error("At least one descriptor type must be used.");
            }

            if(this->nbDescriptorsAnalysis != 1 && this->descriptorName == "programLines"){
                throw std::runtime_error("Only one analysis for program lines descriptors");
            }


            // absMean should only be used alone
            if(useAbsMeanDescriptor && (useMeanDescriptor || useMedianDescriptor ||
                useQuantileDescriptor || useMinMaxDescriptor)){
                std::cerr<<" Warning: useAbsMeanDescriptor is set to true with other ones, since it range between 0, and 1 it will be less effective"<<std::endl;
            }

            this->nbMainDescriptors = useMainMeanDescriptor + useMainMedianDescriptor +
                useMainStdDescriptor + useMainMaxDescriptor + useMainMinDescriptor;
            if(this->nbMainDescriptors > 0){
                this->nbDescriptors = this->nbMainDescriptors;
            }

        }
};

class MapElitesArchive {
    protected:
        const ArchiveParametrization& archiveParams;
        uint64_t dim1;
        uint64_t dim2;
        std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>> archive;

    public:

        MapElitesArchive(const ArchiveParametrization& archiveParams)
            : archiveParams{archiveParams}, dim1{archiveParams.archiveLimits.size()}, dim2{archiveParams.nbDescriptors * archiveParams.nbDescriptorsAnalysis}
        {
            if(dim1 > 0 && dim2 > 0){
                archive.resize(std::pow(dim1, dim2));
            }
        }

        const ArchiveParametrization& getArchiveParams() {return archiveParams;}

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