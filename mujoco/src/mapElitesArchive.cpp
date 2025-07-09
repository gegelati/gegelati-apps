#include "mapElitesArchive.h"


uint64_t MapElitesArchive::size() const
{
    return archive.size();
}

std::pair<uint64_t, uint64_t> MapElitesArchive::getDimensions() const
{
    return std::make_pair(dim1, dim2);
}

const std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>>& MapElitesArchive::getAllArchive() const
{
    return archive;
}

uint64_t MapElitesArchive::getIndexArchive(double value) const
{
    uint64_t idx = 0;
    while (idx < archiveLimits.size() && value > archiveLimits[idx]) {
        idx++;
    }
    return idx >= dim1 ? dim1 - 1 : idx;
}

uint64_t MapElitesArchive::computeLinearIndex(const std::vector<uint64_t>& indices) const
{
    uint64_t index = 0;
    uint64_t multiplier = 1;

    for (int i = dim2 - 1; i >= 0; --i) {
        index += indices[i] * multiplier;
        multiplier *= dim1;
    }

    return index;
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
MapElitesArchive::getArchiveFromDescriptors(const std::vector<double>& descriptors) const
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < dim2; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    return archive[computeLinearIndex(indices)];
}

void MapElitesArchive::setArchiveFromDescriptors(
    const TPG::TPGVertex* vertex,
    std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    std::vector<uint64_t> indices;
    for (uint64_t i = 0; i < dim2; ++i) {
        indices.push_back(getIndexArchive(descriptors[i]));
    }

    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
    MapElitesArchive::getArchiveAt(const std::vector<uint64_t>& indices) const
{
    return archive[computeLinearIndex(indices)];
}

void MapElitesArchive::setArchiveAt(
    const TPG::TPGVertex* vertex,
    std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<uint64_t>& indices)
{
    archive[computeLinearIndex(indices)] = std::make_pair(eval, vertex);
}


void MapElitesArchive::initCSVarchive(std::string path) const {
    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        std::cerr << "Archive file could not be created " << path << std::endl;
        return;
    }

    outFile << "generation";

    std::vector<size_t> indices(dim2, 0);
    size_t total = std::pow(dim1, dim2);
    for (size_t count = 0; count < total; ++count) {

        std::string key;
        for (size_t i = 0; i < dim2; ++i) {
            key += std::to_string(indices[i]);
            if (i != dim2 - 1)
                key += "_";
        }
        outFile << "," << key;


        for (int i = dim2 - 1; i >= 0; --i) {
            if (++indices[i] < dim1)
                break;
            indices[i] = 0;
        }
    }

    outFile << ",archiveRange\n";
    outFile.close();
}

void MapElitesArchive::updateCSVArchive(std::string path, size_t generationNumber) const {
    std::ofstream outFile(path, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Archive file not found " << path << std::endl;
        return;
    }

    outFile << generationNumber;

    std::vector<size_t> indices(dim2, 0);
    size_t total = std::pow(dim1, dim2);
    for (size_t count = 0; count < total; ++count) {
        const auto& elem = archive[computeLinearIndex(indices)];

        if (elem.second != nullptr) {
            outFile << "," << elem.first->getResult();
        } else {
            outFile << ",nan";
        }

        for (int i = dim2 - 1; i >= 0; --i) {
            if (++indices[i] < dim1)
                break;
            indices[i] = 0;
        }
    }

    if (generationNumber == 0) {
        outFile << ",";
        for (size_t i = 0; i < archiveLimits.size(); ++i) {
            outFile << archiveLimits[i];
            if (i != archiveLimits.size() - 1)
                outFile << ";";
        }
    }

    outFile << "\n";
    outFile.close();
}