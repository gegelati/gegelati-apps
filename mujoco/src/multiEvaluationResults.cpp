
#include "multiEvaluationResults.h"

std::map<std::vector<size_t>, double> Learn::MultiEvaluationResult::getScores() const
{
    return this->scores;
}


std::map<std::vector<size_t>, double> Learn::MultiEvaluationResult::getSuccess() const
{
    return this->success;
}

Learn::EvaluationResult& Learn::MultiEvaluationResult::operator+=(
    const Learn::EvaluationResult& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between MultiEvaluationResults.");
    }

    // If the added type is Learn::EvaluationResult
    if (thisType == typeid(Learn::MultiEvaluationResult)) {

        auto* lexiOther = dynamic_cast<const Learn::MultiEvaluationResult*>(&other);

        // Weighted addition of results
        this->result = this->result * (double)this->nbEvaluation +
                       lexiOther->result * (double)lexiOther->nbEvaluation;
        this->result /= (double)this->nbEvaluation + (double)lexiOther->nbEvaluation;

        // Weighted addition of utility
        this->utility = this->utility * (double)this->nbEvaluation +
                        lexiOther->utility * (double)lexiOther->nbEvaluation;
        this->utility /=
            (double)this->nbEvaluation + (double)lexiOther->nbEvaluation;

        // Addition ot nbEvaluation
        this->nbEvaluation += lexiOther->nbEvaluation;

        auto it1 = this->scores.begin();
        while (it1 != this->scores.end()) {
            auto it2 = lexiOther->scores.find(it1->first);

            if (it2 != lexiOther->scores.end()) {
                it1->second = (it1->second * (double)this->nbEvaluation +
                            it2->second * (double)lexiOther->nbEvaluation) /
                            (double)(this->nbEvaluation + lexiOther->nbEvaluation);
            }
            it1++;
        }

        it1 = this->success.begin();
        while (it1 != this->success.end()) {
            auto it2 = lexiOther->success.find(it1->first);

            if (it2 != lexiOther->success.end()) {
                it1->second = (it1->second * (double)this->nbEvaluation +
                            it2->second * (double)lexiOther->nbEvaluation) /
                            (double)(this->nbEvaluation + lexiOther->nbEvaluation);
            }
            it1++;
        }
    }

    return *this;
}
