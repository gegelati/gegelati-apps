

#ifndef LEXICASE_SELECTOR_H
#define LEXICASE_SELECTOR_H

#include "gegelati.h"
#include "multiEvaluationResults.h"
#include "mujocoEnvironment/mujocoWrapper.h"



namespace Selector {

    /**
     * \brief Selection class that will do a selection with a truncation. This
     * is the classic selection methods used in the earliest works of TPGs
     */
    class LexicaseSelector : public virtual Selector
    {

      protected: 
        std::vector<std::vector<size_t>> testCases;

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
            : Selector{graph, params}, testCases{testCases} {
            }

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

        virtual std::vector<std::vector<size_t>> shuffleTestCases(Mutator::RNG& rng) const;
        
        /**
         * \brief select the test cases for the new generation
         */
        //virtual void updateTestCases(Mutator::RNG& rng, Learn::LearningMode mode);


        /**
         * \brief return the current test cases
         */
        const std::vector<std::vector<size_t>>& getCurrentTestCases() const;


        /** */

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





#endif // LEXICASE_SELECTOR_H