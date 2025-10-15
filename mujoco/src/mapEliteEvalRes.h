

#ifndef MAP_ELITE_EVAL_RES_H
#define MAP_ELITE_EVAL_RES_H

#include <gegelati.h>
#include "descriptors.h"


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
         * \brief Map storing a descriptor and a Vector storing a double value per descriptor (i.e. per Action) of
         * a classification LearningEnvironment.
         */
        std::map<Descriptor::DescriptorType, std::vector<double>> mapDescriptors;

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
            const double& res, const size_t& nbEval, const double& uti,
            const std::map<Descriptor::DescriptorType, std::vector<double>>& mapDescriptors)
            : EvaluationResult(res, nbEval, uti), mapDescriptors{mapDescriptors}
        {}

        /**
         * \brief Get a const ref to the scorePerClass attribute.
         */
        const std::map<Descriptor::DescriptorType, std::vector<double>>& getMapDescriptors() const;

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