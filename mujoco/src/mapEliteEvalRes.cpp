
#include "mapEliteEvalRes.h"

const std::vector<double>& Learn::MapElitesEvaluationResult::getDescriptors() const
{
    return descriptors;
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
    if (otherConverted.descriptors.size() != this->descriptors.size()) {
        throw std::runtime_error(
            "Size mismatch between AdversarialEvaluationResults.");
    }

    // Weighted addition of descriptors
    for (size_t i = 0; i < descriptors.size(); i++) {
        this->descriptors[i] =
            this->descriptors[i] * (double)this->nbEvaluation +
            otherConverted.descriptors[i] * (double)otherConverted.nbEvaluation;
        this->descriptors[i] /=
            (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;
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