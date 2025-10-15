

#include "mapElitesMutator.h"


void Mutator::MapElitesMutator::addRandomActionEdge(
    TPG::TPGGraph& graph, const TPG::TPGAction& action,
    const std::vector<const TPG::TPGAction*>& preExistingActions,
    Mutator::RNG& rng)
{



    // Pick an edge (excluding ones from the team and edges with the team as a
    // destination)
    std::list<const TPG::TPGEdge*> pickableEdges;
    const std::set<uint64_t>& assessedActions = action.getAssessedActions();

    // For each Action
    for(const TPG::TPGAction* existingAction: preExistingActions){

        // For each edge
        for(auto edge: existingAction->getOutgoingEdges()){

            // If the action value is not assessed by the action node, add the edge
            if(assessedActions.find(dynamic_cast<const TPG::TPGActionEdge*>(edge)->getActionClass()) == assessedActions.end()){
                pickableEdges.push_back(edge);
            }
        }
    }

    

    if(pickableEdges.size() == 0){
        // Chances are really low but the pickableEdges can be empty
        return;
    }

    // Pick a pickable Edge
    // (This code assumes that the set of pickable edge is never empty..
    // otherwise it will throw an exception. Possible solution if needed
    // initialize an entirely new program and pick a random target.)
    std::list<const TPG::TPGEdge*>::iterator iter = pickableEdges.begin();
    std::advance(iter, rng.getUnsignedInt64(0, pickableEdges.size() - 1));
    const TPG::TPGEdge* pickedEdge = *iter;



    // Create new edge from team and with the same ProgramSharedPointer
    // But with the team as its source
    // throw std::runtime_error if the edge is not from the graph;
    const TPG::TPGEdge& newEdge = graph.cloneEdge(*pickedEdge);
    graph.setEdgeSource(newEdge, action);
}



void Mutator::MapElitesMutator::mutateTPGAction(
    TPG::TPGGraph& graph, const TPG::TPGAction& action,
    const std::vector<const TPG::TPGAction*>& preExistingActions,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, Mutator::RNG& rng)
{ 
    if(params.tpg.useMultiActionProgram){
        // 1. Remove randomly selected edges
        // Keep at least two edges (otherwise the team is useless)
        double proba = params.tpg.pActionEdgeDeletion;
        while (action.getOutgoingEdges().size() > 1 &&
            proba > rng.getDouble(0.0, 1.0)) {
            TPGMutator::removeRandomActionEdge(graph, action, rng);

            // Decrement the proba of removing another edge
            proba *= params.tpg.pActionEdgeDeletion;

            // Update assessed actions    
            graph.updateAssessedActions(&action);
            
        }

        // 2. Add random duplicated edge with the team as its source
        proba = params.tpg.pActionEdgeAddition;
        while (action.getOutgoingEdges().size() < graph.getEnvironment().getNbContinuousActions() &&
            proba > rng.getDouble(0.0, 1.0)) {

            // Add an edge (by duplication of an existing one)
            addRandomActionEdge(graph, action, preExistingActions, rng);

            // Decrement the proba of adding another edge
            proba *= params.tpg.pActionEdgeAddition;

            // Update assessed actions    
            graph.updateAssessedActions(&action);
                    
        }

        // 3. swap randomly selected edges
        // With at least two edges
        proba = params.tpg.pSwapActionProgram;
        while (action.getOutgoingEdges().size() > 2 &&
                proba > rng.getDouble(0.0, 1.0)) {
            TPGMutator::swapActionEdges(graph, action, rng);

            // Decrement the proba of swapping two edges
            proba *= params.tpg.pSwapActionProgram;
        }
    }
 
 

    bool anyMutationDone = false;
    do {
        std::vector<uint64_t> indexUsed;
        uint64_t index;
        // 4. mutate randomly selected program on action Edge. 
        double proba = params.tpg.pMutateActionProgram;
        while(indexUsed.size() < action.getOutgoingEdges().size()  && proba > rng.getDouble(0.0, 1.0)){

            do {
                index = rng.getUnsignedInt64(0, action.getOutgoingEdges().size()-1);
            } while(std::find(indexUsed.begin(), indexUsed.end(), index) != indexUsed.end()) ;

            indexUsed.push_back(index);
    
            std::list<TPG::TPGEdge *>::const_iterator iter = action.getOutgoingEdges().begin();
            std::advance(iter, index);
            TPG::TPGActionEdge* actionEdge = dynamic_cast<TPG::TPGActionEdge*>(*iter);

            TPGMutator::mutateTPGActionEdge(graph, action, actionEdge, newPrograms, params, rng);

            proba *= params.tpg.pMutateActionProgram;

            anyMutationDone = true;
        }
    } while (!anyMutationDone && params.tpg.pMutateActionProgram != 0.0);


    graph.orderActionEdges(&action);

}

std::vector<const TPG::TPGAction*> Mutator::MapElitesMutator::getPreExistingActions(
    const std::map<Descriptor::DescriptorType, MapElitesArchive*>& mapElitesArchives)
{

    std::vector<const TPG::TPGAction*> preExistingActions;
    std::set<const TPG::TPGAction*> setPreExistingActions;
    for(auto pairDescriptor: mapElitesArchives){
        auto& archive = pairDescriptor.second->getAllArchive();
        for(auto& pairElem: archive){
            if (pairElem.second != nullptr) {
                if(dynamic_cast<const TPG::TPGAction*>(pairElem.second) == nullptr){
                    throw std::runtime_error("Element should be an action");
                }
                size_t currentSetSize = setPreExistingActions.size();
                setPreExistingActions.insert(dynamic_cast<const TPG::TPGAction*>(pairElem.second));

                // The size change, meaning that the element was not already in the set.
                if(currentSetSize != setPreExistingActions.size()){
                    preExistingActions.push_back(dynamic_cast<const TPG::TPGAction*>(pairElem.second));
                }
            }
        }
    }

    return preExistingActions;
}

void Mutator::MapElitesMutator::populateTPG(TPG::TPGGraph& graph,
                                      const Archive& archive,
                                      const std::map<Descriptor::DescriptorType, MapElitesArchive*>& mapElitesArchives,
                                      const Mutator::MutationParameters& params,
                                      Mutator::RNG& rng, uint64_t nbGenerations,
                                      uint64_t maxNbThreads)
{
    if(nbGenerations == 0){
        return;
    }


    // Create an empty list to store Programs to mutate.
    std::list<std::shared_ptr<Program::Program>> newPrograms;
    uint64_t nbRootsToCreate = params.tpg.nbRoots;
    std::vector<const TPG::TPGAction*> preExistingVertices = getPreExistingActions(mapElitesArchives);
    uint64_t nbRootsCreated = 0;
    while (nbRootsCreated < nbRootsToCreate) {
        size_t reduc = 1;
        if(preExistingVertices.size() > 1){
            reduc = 2;
        }

        // Select a random existing root in the archive
        size_t index1 = rng.getUnsignedInt64(0, preExistingVertices.size() - 1);
        size_t index2 = rng.getUnsignedInt64(0, preExistingVertices.size() - reduc);
        if(index1 == index2 && preExistingVertices.size() > 1){
            index2++;
        }

        // Clone the agents
        const TPG::TPGAction* child1 = (const TPG::TPGAction*)(&graph.cloneVertex(*preExistingVertices.at(index1)));
        const TPG::TPGAction* child2 = (const TPG::TPGAction*)(&graph.cloneVertex(*preExistingVertices.at(index2)));
        // Get parents and create childs
        std::vector<const TPG::TPGAction*> childs{child1, child2};
        
        if(index1 != index2){
            // Do the crossover over the childs
            Mutator::TPGMutator::crossTPGAction(graph, childs, params, rng);
        }
        // Then do the mutation over the childs
        if(child1->getOutgoingEdges().size() != 0){
            mutateTPGAction(graph, *child1, preExistingVertices, newPrograms,
                params, rng);
                nbRootsCreated++;
        } else {
            graph.removeVertex(*child1);
        }
        if(child2->getOutgoingEdges().size() != 0){
            mutateTPGAction(graph, *child2, preExistingVertices, newPrograms,
                params, rng);
                nbRootsCreated++;
        } else {
            graph.removeVertex(*child2);
        }



    }


    // Mutate the new Programs
    TPGMutator::mutateNewProgramBehaviors(maxNbThreads, newPrograms, rng, params, archive);
}
