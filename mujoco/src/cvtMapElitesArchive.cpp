
#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <cmath>
#include <unordered_map>
#include <numeric>

#include "cvtMapElitesArchive.h"

double CvtMapElitesArchive::dist_squared(const std::vector<double>& a, const std::vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < dim2; ++i)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return sum;
}

std::vector<double> CvtMapElitesArchive::add(const std::vector<double>& a, const std::vector<double>& b) {
    std::vector<double> res(dim2);
    for (size_t i = 0; i < dim2; ++i)
        res[i] = a[i] + b[i];
    return res;
}

std::vector<double> CvtMapElitesArchive::scalar_mult(const std::vector<double>& a, double s) {
    std::vector<double> res(dim2);
    for (size_t i = 0; i < dim2; ++i)
        res[i] = a[i] * s;
    return res;
}

std::vector<double> CvtMapElitesArchive::average(const std::vector<std::vector<double>>& points) {
    std::vector<double> avg(dim2, 0.0);
    if (points.empty()) return avg;
    for (const auto& p : points)
        avg = add(avg, p);
    return scalar_mult(avg, 1.0 / points.size());
}

std::vector<double> CvtMapElitesArchive::random_point(Mutator::RNG& rng) {
    std::vector<double> point(dim2);
    for (size_t i = 0; i < dim2; ++i)
        point[i] = rng.getDouble(archiveParams.minRange, archiveParams.maxRange);
    return point;
}

size_t CvtMapElitesArchive::nearest(const std::vector<double>& point, const std::vector<std::vector<double>>& centroids) {
    double best_dist = std::numeric_limits<double>::max();
    int64_t best_idx = -1;
    for (size_t i = 0; i < centroids.size(); ++i) {
        double d = dist_squared(point, centroids[i]);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }
    if(best_idx == -1){
        throw std::runtime_error("Should have found a best centroid");
    }
    return best_idx;
}

void CvtMapElitesArchive::initialize_cvt(Mutator::RNG& rng)
{
    for (auto& c : centroids)
        c = random_point(rng);

    std::vector<size_t> j(archiveParams.nbCentroids, 1); // Compteur d'updates par centroïde

    for (size_t iter = 0; iter < archiveParams.nbIterationInit; ++iter) {

        // print progress with a line overrite at each iteration
        std::cout << "\rIteration " << iter + 1 << "/" << archiveParams.nbIterationInit << " (centroids initialized: " << j.size() << ")"<< std::flush;
        std::vector<std::vector<double>> samples(archiveParams.nbDotsInit);
        std::vector<std::vector<std::vector<double>>> assignments(archiveParams.nbCentroids);

        // Create random dots
        for (size_t i = 0; i < archiveParams.nbDotsInit; ++i)
            samples[i] = random_point(rng);

        // Assign each dot to the closest centroid
        for (const auto& p : samples) {
            size_t idx = nearest(p, centroids);
            assignments[idx].push_back(p);
        }

        // Update the centroids
        for (size_t i = 0; i < archiveParams.nbCentroids; ++i) {
            if (!assignments[i].empty()) {
                std::vector<double> u_i = average(assignments[i]);
                std::vector<double> z_i = centroids[i];
                size_t j_i = j[i];

                // Update
                std::vector<double> new_z(dim2);
                for (size_t d = 0; d < dim2; ++d) {
                    new_z[d] = ((archiveParams.a1 * j_i + archiveParams.b1) * z_i[d] + (archiveParams.a2 * j_i + archiveParams.b2) * u_i[d]) / (j_i + 1);
                }

                centroids[i] = new_z;
                j[i]++;
            }
        }
    }
    std::cout<<std::endl;  
}

void CvtMapElitesArchive::initCSVarchive(std::string archivePath) const {
    // 1. Archive file (header with indices)
    std::ofstream outArchive(archivePath);
    if (!outArchive.is_open()) {
        std::cerr << "Archive file could not be created: " << archivePath << std::endl;
        return;
    }

    outArchive << "generation";
    for (size_t i = 0; i < centroids.size(); ++i) {
        outArchive << "," << i;  // Use centroid ID as column name
    }
    outArchive << "\n";
    outArchive.close();

    // Create centroidsPath by replacing "archive" with "centroids" in archivePath
    std::string centroidsPath = archivePath;
    size_t pos = centroidsPath.find("archive");
    if (pos != std::string::npos) {
        centroidsPath.replace(pos, 7, "centroids");
    }

    // 2. Centroids file (centroid coordinates)
    std::ofstream outCentroids(centroidsPath);
    if (!outCentroids.is_open()) {
        std::cerr << "Centroids file could not be created: " << centroidsPath << std::endl;
        return;
    }

    outCentroids << "centroid_id";
    for (size_t d = 0; d < centroids[0].size(); ++d) {
        outCentroids << ",dim" << d;
    }
    outCentroids << "\n";

    for (size_t i = 0; i < centroids.size(); ++i) {
        outCentroids << i;
        for (double val : centroids[i]) {
            outCentroids << "," << val;
        }
        outCentroids << "\n";
    }

    outCentroids.close();
}


void CvtMapElitesArchive::updateCSVArchive(std::string path, uint64_t generationNumber) const {
    std::ofstream outFile(path, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Archive file not found: " << path << std::endl;
        return;
    }

    outFile << generationNumber;

    for (size_t i = 0; i < archive.size(); ++i) {
        const auto& elem = archive[i];
        if (elem.second != nullptr) {
            outFile << "," << elem.first->getResult();
        } else {
            outFile << ",nan";
        }
    }

    outFile << "\n";
    outFile.close();
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& 
CvtMapElitesArchive::getArchiveFromDescriptors(const std::vector<double>& descriptors) const
{
    size_t idx = getIndexForDescriptor(descriptors);
    return archive[idx];
}

void CvtMapElitesArchive::setArchiveFromDescriptors(
    const TPG::TPGVertex* vertex,
    std::shared_ptr<Learn::EvaluationResult> eval,
    const std::vector<double>& descriptors)
{
    size_t idx = getIndexForDescriptor(descriptors);
    archive[idx] = std::make_pair(eval, vertex);
}

size_t CvtMapElitesArchive::getIndexForDescriptor(const std::vector<double>& descriptors) const {
    size_t bestIndex = 0;
    double bestDist = std::numeric_limits<double>::max();

    for (size_t i = 0; i < centroids.size(); ++i) {
        double dist = 0.0;
        for (size_t d = 0; d < descriptors.size(); ++d) {
            double diff = descriptors[d] - centroids[i][d];
            dist += diff * diff;
        }
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = i;
        }
    }


    return bestIndex;
}
