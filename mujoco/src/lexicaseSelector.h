

#ifndef LEXICASE_SELECTOR_H
#define LEXICASE_SELECTOR_H

#include "gegelati.h"



namespace Selector {

    /**
     * \brief Selection class that will do a selection with a truncation. This
     * is the classic selection methods used in the earliest works of TPGs
     */
    class LexicaseSelector : public Selector
    {

      protected: 
        std::vector<std::vector<size_t>> allTestCases;// = {
            //{1}, {2}//, {3}, {4}, {0},
            //{1,2}, {1,3}, {1,4}, {1,0}, {2,3}, {2,4}, {2,0}, {3,4}, {3,0}, {4,0},
            //{1,2,3}, {1,2,4}, {1,2,0}, {1,3,4}, {1,3,0}, {1,4,0}, {2,3,4}, {2,3,0}, {2,4,0}, {3,4,0},
            //{1,2,3,4}, {1,2,3,0}, {1,2,4,0}, {1,3,4,0}, {2,3,4,0},
            //{1,2,3,4,0}
        //};

        std::vector<std::vector<size_t>> currentTestCases;

      public:
        /**
         * \brief Constructor for Selector.
         *
         * \param[in] graph shared pointer of the graph on which the selection
         * is done. \param[in] params parameters used by the Selector.
         */
        LexicaseSelector(std::shared_ptr<TPG::TPGGraph> graph,
                         const Learn::LearningParameters& params,
                         const std::vector<std::vector<size_t>>& testCases)
            : Selector{graph, params}, allTestCases{testCases} {}

        /**
         * \brief override of doSelection method
         *
         * Removed the worst agents from the population with a truncation
         * process where the worst proportion set in the parameters is deleted.
         *
         * \param[in,out] results a multimap containing root TPGVertex
         * associated to their score during an evaluation.
         * \param[in] rng Random Number Generator used in the mutation process.
         */
        virtual void doSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            Mutator::RNG& rng) override;

        virtual std::vector<std::vector<size_t>> shuffleTestCases(Mutator::RNG& rng);
        
        /**
         * \brief select the test cases for the new generation
         */
        virtual void updateTestCases(Mutator::RNG& rng, Learn::LearningMode mode);


        /**
         * \brief return the current test cases
         */
        const std::vector<std::vector<size_t>>& getCurrentTestCases() const;


        /**
         * \brief add a vertex to the verticesToDelete set.
         *
         * \param[in] vertex TPGVertex added to the vertices to remove
         */
        //void addToVerticesToDelete(const TPG::TPGVertex* vertex);

        /**
         * \brief Specialization of updateContext for tournament purposes
         *
         * The method will remove the elite agents from the clonableVertices
         * vectors, and will remove the not elite agents from the
         * preExistingVertices vectors
         */
        //virtual const SelectionContext& updateContext() override;

        /**
         * \brief Specialization of deleteUselessParents for tournament purposes
         *
         * This method erase the agents that have survived the tournaments and
         * have generated new offsprings.
         */
        //virtual void deleteUselessParents() override;

        /**
         * \brief getter of the verticesToDelete set.
         */
        //virtual const std::set<const TPG::TPGVertex*>& getVerticesToDelete();
    };
}; // namespace Selector




namespace Learn {
    class LexicaseAgent : public ParallelLearningAgent
    {
        public:
            LexicaseAgent(
                LearningEnvironment& le, const Instructions::Set& iSet,
                const LearningParameters& p, const std::vector<std::vector<size_t>>& testCases,
                const TPG::TPGFactory& factory = TPG::TPGFactory())
                : ParallelLearningAgent(le, iSet, p, factory)
            {
                // There is probably a cleaner way to do that, but using the factory
                // was creating import issues.
                if (p.selection.selectionMode == "truncation") {
                    selector =
                        std::make_shared<Selector::TruncationSelector>(tpg, p);
                }
                else if (p.selection.selectionMode == "tournament") {
                    selector =
                        std::make_shared<Selector::TournamentSelector>(tpg, p);
                }
                else if (p.selection.selectionMode == "lexicase"){
                    selector = std::make_shared<Selector::LexicaseSelector>(tpg, p, testCases);
                } else {
                    throw std::runtime_error("Selection mode not found");
                }
            }

            virtual std::shared_ptr<EvaluationResult> evaluateJob(
                TPG::TPGExecutionEngine& tee, const Job& job,
                uint64_t generationNumber, LearningMode mode,
                LearningEnvironment& le) const override;


            virtual void trainOneGeneration(uint64_t generationNumber,
                                            bool doPopulate = true) override;


    };

    class LexicaseEvaluationResult : public EvaluationResult
    {
        protected:
            std::map<std::vector<size_t>, double> scores;

        public: 
            LexicaseEvaluationResult(const std::map<std::vector<size_t>, double>& scores, const double& res, const size_t& nbEval,
                            const double& uti = std::nan(""))
            : EvaluationResult {res, nbEval, uti}, scores{scores} {};
            
            
            /**
             * \brief Virtual method to get the default double equivalent of
             * the reward of the EvaluationResult.
             */
            virtual std::map<std::vector<size_t>, double> getScores() const;
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

#endif // LEXICASE_SELECTOR_H