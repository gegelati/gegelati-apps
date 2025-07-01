

#ifndef MAP_ELITE_EVAL_RES_H
#define MAP_ELITE_EVAL_RES_H

#include <gegelati.h>


namespace Learn {
    /**
     * \brief Specialization of the EvaluationResult class for classification
     * LearningEnvironment.
     *
     * The main difference with the base EvaluationResult class is that a score
     * per Action of a learning environment can be stored within this class.
     */
    class MapElitesEvaluationResult : public EvaluationResult
    {
      protected:
        /**
         * \brief Vector storing a double score per class (i.e. per Action) of
         * a classification LearningEnvironment.
         */
        std::vector<double> feetContact;

      public:
        /**
         * \brief Main constructor of the ClassificationEvaluationResult class.
         *
         * A ClassificationEvaluationResult storing a score for each class of a
         * classification-oriented LearningEnvironment.
         *
         * Contrary to the base class EvaluationResult, the number of evaluation
         * stored in a ClassificationEvaluationResult corresponds to the total
         * number of times any action was performed.
         *
         * \param[in] scores a vector of double storing per-class scores.
         * \param[in] nbEvalPerClass a vector of integer storing per-class
         * number of evaluations.
         */
        MapElitesEvaluationResult(
            const double& res, const double& uti, const size_t& nbEval,
            const std::vector<double>& feetContact)
            : EvaluationResult(res, nbEval, uti), feetContact{feetContact}
        {}

        /**
         * \brief Get a const ref to the scorePerClass attribute.
         */
        const std::vector<double>& getFeetContact() const;

        /**
         * \brief Override from EvaluationResult
         *
         * \throw std::runtime_error in case the number of classes of the two
         * ClassificationEvaluationResult are different.
         */
        virtual EvaluationResult& operator+=(
            const EvaluationResult& other) override;
    };
}; // namespace Learn

#endif