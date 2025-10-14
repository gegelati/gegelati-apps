
#include "hierachicalSelector.h"

void Selector::HierarchicalSelector::launchSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                    const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{
    // Clear the set of vertices to delete.
    this->verticesToDelete.clear();
    // Preparing multi-population selection....
    if (params.mutation.tpg.ratioTeamsOverActions != 0.0 &&
        params.mutation.tpg.ratioTeamsOverActions != 1.0) {

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      const TPG::TPGVertex*>
            resultsTeam;
        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      const TPG::TPGVertex*>
            resultsAction;

        // Split the results into result of team and result of action
        for (const auto& p : results) {
            if (dynamic_cast<const TPG::TPGAction*>(p.second)){

                resultsAction.insert(p);
            }
            else {
                resultsTeam.insert(p);
            }
        }

        // --- SPLIT actions by testCase ---
        // Suppose allTestCases.size() == nb d'actions distinctes
        std::vector<std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>> resultsActionPerTestCase;
        resultsActionPerTestCase.resize(this->testCases.size());

        for (const auto& p : resultsAction) {
            auto action = dynamic_cast<const TPG::TPGAction*>(p.second);
            if (action) {
                size_t id = action->getActionID();
                if (id < resultsActionPerTestCase.size()) {
                    resultsActionPerTestCase[id].insert(p);
                }
            }
        }

        

        
        //std::cout<<std::endl;
        //std::cout<<"Number of action roots "<< graph->getRootActions().size()<< "And number of roots" << graph->getNbRootVertices()<<std::endl;

        // Sélection tournoi spécifique à chaque testCase
        for (auto& actionGroup : resultsActionPerTestCase) {
            if (!actionGroup.empty()) {
                TournamentSelector::doSelection(actionGroup, rng);
                //std::cout<<"Number of action roots "<< graph->getRootActions().size()<< "And number of roots" << graph->getNbRootVertices()<< " and number of vertices to delete " << this->getVerticesToDelete().size()<<std::endl;;
            }
        }

        // Fusionner tous les sous-groupes d'actions sélectionnés
        resultsAction.clear();
        for (const auto& actionGroup : resultsActionPerTestCase) {
            resultsAction.insert(actionGroup.begin(), actionGroup.end());
        }

        // Sélection lexicase sur les teams
        if(params.selection._selectionMode == "lexicase"){
            LexicaseSelector::doSelection(resultsTeam, rng);
        } else if (params.selection._selectionMode == "tournament"){
            TournamentSelector::doSelection(resultsTeam, rng);
        } else {
            throw std::runtime_error("Selection mode not recognized in HierarchicalSelector");
        }

        // Fusing the results
        results.clear();
        results.insert(resultsTeam.begin(), resultsTeam.end());
        results.insert(resultsAction.begin(), resultsAction.end());

    }
    else {
        this->doSelection(results, rng);
    }
}

void Selector::HierarchicalSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                    const TPG::TPGVertex*>& results,
    Mutator::RNG& rng)
{
    throw std::runtime_error("Hierarchical selection not implemented yet");
}
