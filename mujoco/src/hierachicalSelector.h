

#ifndef HIERARCHICAL_SELECTOR_H
#define HIERARCHICAL_SELECTOR_H

#include "gegelati.h"
#include "lexicaseSelector.h"
#include "multiEvaluationResults.h"

namespace Selector
{
    
    class HierarchicalSelector: public virtual LexicaseSelector, public virtual TournamentSelector
    {
        public:
            HierarchicalSelector(std::shared_ptr<TPG::TPGGraph> graph,
                         const Learn::LearningParameters& params,
                         const std::vector<std::vector<size_t>>& testCases)
            : Selector{graph, params}, LexicaseSelector{graph, params, testCases}, TournamentSelector{graph, params} {}

            
        virtual void launchSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            Mutator::RNG& rng) override;

        virtual void doSelection(
            std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                          const TPG::TPGVertex*>& results,
            Mutator::RNG& rng) override;

        virtual const SelectionContext& updateContext() override {
            return TournamentSelector::updateContext();
        }
    };
}

#endif