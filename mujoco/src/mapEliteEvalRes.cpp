
#include "mapEliteEvalRes.h"

const std::vector<double>& Learn::MapElitesEvaluationResult::getFeetContact() const
{
    return feetContact;
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
    if (otherConverted.feetContact.size() != this->feetContact.size()) {
        throw std::runtime_error(
            "Size mismatch between AdversarialEvaluationResults.");
    }

    
    this->result = this->result * (double)this->nbEvaluation +
                    otherConverted.result * (double)otherConverted.nbEvaluation;
    this->result /= (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;


    
    this->utility = this->utility * (double)this->nbEvaluation +
                    otherConverted.utility * (double)otherConverted.nbEvaluation;
    this->utility /= (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;

    // If the added type is Learn::EvaluationResult
    // Weighted addition of results
    for (size_t i = 0; i < feetContact.size(); i++) {
        this->feetContact[i] =
            this->feetContact[i] * (double)this->nbEvaluation +
            otherConverted.feetContact[i] * (double)otherConverted.nbEvaluation;
        this->feetContact[i] /=
            (double)this->nbEvaluation + (double)otherConverted.nbEvaluation;
    }

    // Addition ot nbEvaluation
    this->nbEvaluation += otherConverted.nbEvaluation;

    return *this;
}