

#include "mapElitesMutator.h"


void Mutator::MapElitesMutator::addRandomActionEdge(
    TPG::TPGGraph& graph, const TPG::TPGAction& action,
    const MapElitesArchive& mapElitesArchive,
    const std::vector<uint64_t>& indicesCloned,
    const std::set<std::vector<uint64_t>>& validIndices,
    Mutator::RNG& rng, bool useOnlyCloseAddEdges)
{

    std::vector<std::vector<uint64_t>> pickableIndices;
    std::pair<uint64_t, uint64_t> dim = mapElitesArchive.getDimensions(); // dim.first = dim1, dim.second = dim2

    if(useOnlyCloseAddEdges){
        for (size_t d = 0; d < dim.second; ++d) {
            // -1
            if (indicesCloned[d] > 0) {
                std::vector<uint64_t> neighbor = indicesCloned;
                neighbor[d] -= 1;
                if (std::find(validIndices.begin(), validIndices.end(), neighbor) != validIndices.end()) {
                    pickableIndices.push_back(neighbor);
                }
            }
            // +1
            if (indicesCloned[d] + 1 < dim.first) {
                std::vector<uint64_t> neighbor = indicesCloned;
                neighbor[d] += 1;
                if (std::find(validIndices.begin(), validIndices.end(), neighbor) != validIndices.end()) {
                    pickableIndices.push_back(neighbor);
                }
            }
        }
    } else {
        for (const auto& indices : validIndices) {
            if (indices != indicesCloned) {
                pickableIndices.push_back(indices);
            }
        }
    }


    // Pick an edge (excluding ones from the team and edges with the team as a
    // destination)
    std::list<const TPG::TPGEdge*> pickableEdges;
    const std::set<uint64_t>& assessedActions = action.getAssessedActions();

    // For each index
    for(auto indices: pickableIndices){

        // FOr each edge
        for(auto edge: mapElitesArchive.getArchiveAt(indices).second->getOutgoingEdges()){

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
    const MapElitesArchive& mapElitesArchive,
    const std::vector<uint64_t>& indicesCloned,
    const std::set<std::vector<uint64_t>>& validIndices,
    std::list<std::shared_ptr<Program::Program>>& newPrograms,
    const Mutator::MutationParameters& params, Mutator::RNG& rng,
    bool useOnlyCloseAddEdges)
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
            addRandomActionEdge(graph, action, mapElitesArchive, indicesCloned, validIndices, rng, useOnlyCloseAddEdges);

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

std::pair<std::set<std::vector<size_t>>, std::vector<std::vector<size_t>>> 
    Mutator::MapElitesMutator::getValidAndWeightedIndices(
        const MapElitesArchive& mapElitesArchive, bool usePonderationSelection)
{
    std::vector<std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex *>> copiedArchive;
    std::map<const TPG::TPGVertex *, double> weightPerIndices;

    std::set<std::vector<size_t>> validIndices;
    std::pair<uint64_t, uint64_t> dim = mapElitesArchive.getDimensions();

    std::vector<std::vector<size_t>> weightedIndices;
    std::vector<size_t> indices(dim.second, 0);


    if(usePonderationSelection){
        for (const auto& entry : mapElitesArchive.getAllArchive()) {
            if (entry.second != nullptr) {
                copiedArchive.push_back(entry);
            }
        }

        std::sort(
            copiedArchive.begin(),
            copiedArchive.end(),
            [](const auto& a, const auto& b) {
                return a.first->getResult() < b.first->getResult();
            }
        );

        size_t idx = 1;
        for(const auto& entry: copiedArchive){
            weightPerIndices.insert(std::make_pair(entry.second, idx++));
        }
    }


    for (size_t flatIndex = 0; flatIndex < mapElitesArchive.size(); ++flatIndex) {
        size_t idx = flatIndex;

        // Calcul des indices multidimensionnels à partir de l’index linéaire
        for (int d = dim.second - 1; d >= 0; --d) {
            indices[d] = idx % dim.first;   // modulo par taille de chaque dimension
            idx /= dim.first;
        }


        // Récupération de l’élément dans l’archive
        const auto& elem = mapElitesArchive.getArchiveAt(indices);
        if (elem.second != nullptr) {
            validIndices.insert(indices);
            // Add the element n time if ponderation selection is used
            if(usePonderationSelection){
                for(size_t idx = 0; idx < weightPerIndices.at(elem.second); idx++){
                    weightedIndices.push_back(indices);
                }
            } else {
                weightedIndices.push_back(indices);
            }
        }
    }

    return std::make_pair(validIndices, weightedIndices);
}

void Mutator::MapElitesMutator::populateTPG(TPG::TPGGraph& graph,
                                      const Archive& archive,
                                      const MapElitesArchive& mapElitesArchive,
                                      const Mutator::MutationParameters& params,
                                      Mutator::RNG& rng, uint64_t nbGenerations,
                                      uint64_t maxNbThreads,
                                      bool usePonderationSelection,
                                      bool useOnlyCloseAddEdges)
{

    if(nbGenerations == 0){
        return;
    }

    // Create an empty list to store Programs to mutate.
    std::list<std::shared_ptr<Program::Program>> newPrograms;

    uint64_t nbRootsToCreate = params.tpg.nbRoots;

    auto pair = getValidAndWeightedIndices(mapElitesArchive, usePonderationSelection);
    std::set<std::vector<size_t>> validIndices = pair.first;
    std::vector<std::vector<size_t>> weightedIndices = pair.second ;




    uint64_t nbRootsCreated = 0;
    while (nbRootsCreated < nbRootsToCreate) {

        // Select a random existing root in the archive
        std::vector<uint64_t> indicesCloned = weightedIndices.at(rng.getUnsignedInt64(0, weightedIndices.size() - 1));

        // Clone the agent
        const TPG::TPGAction* child = (const TPG::TPGAction*)(&graph.cloneVertex(
            *mapElitesArchive.getArchiveAt(indicesCloned).second
        ));
        
        mutateTPGAction(graph, *child, mapElitesArchive, indicesCloned, validIndices, newPrograms,
            params, rng, useOnlyCloseAddEdges);



        // Check the new number of roots
        // Needed since preExisting root may be subsumed by new ones.
        nbRootsCreated++;
    }


    // Mutate the new Programs
    TPGMutator::mutateNewProgramBehaviors(maxNbThreads, newPrograms, rng, params, archive);
}
