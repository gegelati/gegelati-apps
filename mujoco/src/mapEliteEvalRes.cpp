
#include "mapEliteEvalRes.h"

const std::map<Descriptor::DescriptorType, std::vector<double>>& Learn::MapElitesEvaluationResult::getMapDescriptors() const
{
    return this->mapDescriptors;
}


Learn::EvaluationResult& Learn::MapElitesEvaluationResult::operator+=(
    const EvaluationResult& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between EvaluationResults.");
    }

    auto otherConverted = (const MapElitesEvaluationResult&)other;

    // Size Check
    if (otherConverted.mapDescriptors.size() != this->mapDescriptors.size()) {
        throw std::runtime_error(
            "Size mismatch between Rresults.");
    }

    // Weighted addition of descriptors
    for(auto& pairDescriptor: mapDescriptors){
        for (size_t i = 0; i < pairDescriptor.second.size(); i++) {
            pairDescriptor.second[i] =
                pairDescriptor.second[i] * (double)this->nbEvaluation +
                otherConverted.mapDescriptors.at(pairDescriptor.first)[i] * (double)otherConverted.nbEvaluation;
            pairDescriptor.second[i] /=
                (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;
        }
    }


    this->result = this->result * (double)this->nbEvaluation +
                    otherConverted.result * (double)otherConverted.nbEvaluation;
    this->result /= (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;

    // Weighted addition of utility
    this->utility = this->utility * (double)this->nbEvaluation +
                    otherConverted.utility * (double)otherConverted.nbEvaluation;
    this->utility /= (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;


    // Addition ot nbEvaluation
    this->nbEvaluation += otherConverted.nbEvaluation;

    return *this;
}