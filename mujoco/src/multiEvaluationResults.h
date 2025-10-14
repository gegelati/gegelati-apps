


#ifndef MULTI_EVALUATION_RESULTS_H
#define MULTI_EVALUATION_RESULTS_H

#include <gegelati.h>

namespace Learn {
   

    class MultiEvaluationResult : public EvaluationResult
    {
        protected:
            std::map<std::vector<size_t>, double> scores;
            std::map<std::vector<size_t>, double> success;

        public: 
            MultiEvaluationResult(const std::map<std::vector<size_t>, double>& scores, const std::map<std::vector<size_t>, double>& success, const double& res, const size_t& nbEval,
                            const double& uti = std::nan(""))
            : EvaluationResult {res, nbEval, uti}, scores{scores}, success{success} {};
            
            
            /**
             * \brief Virtual method to get the default double equivalent of
             * the score of the EvaluationResult.
             */
            virtual std::map<std::vector<size_t>, double> getScores() const;
            /**
             * \brief Virtual method to get the default double equivalent of
             * the success of the EvaluationResult.
             */
            virtual std::map<std::vector<size_t>, double> getSuccess() const;
            /**
             * \brief Polymorphic addition assignement operator for
             * EvaluationResult.
             *
             * \throw std::runtime_error in case the other EvaluationResult and
             * this have a different typeid.
             */
            virtual EvaluationResult& operator+=(const EvaluationResult& other) override;
            
    };
};

#endif