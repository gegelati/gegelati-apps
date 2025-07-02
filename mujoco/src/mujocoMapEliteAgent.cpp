
#include "mujocoMapEliteAgent.h"
#include "mapElitesMutator.h"
#include "mapEliteEvalRes.h"
#include "mujocoEnvironment/mujocoAntWrapper.h"

void Learn::MujocoMapEliteLearningAgent::trainOneGeneration(uint64_t generationNumber)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Populate Sequentially
    if(mapEliteArchive.size() > 0){
        Mutator::MapElitesMutator::populateTPG(
            *this->tpg, this->archive, this->mapEliteArchive, this->params.mutation, this->rng,
            generationNumber, maxNbThreads, usePonderationSelection, useOnlyCloseAddEdges);
    } else {
        Mutator::TPGMutator::populateTPG(
            *this->tpg, this->archive, this->params.mutation, this->rng,
            generationNumber, maxNbThreads);
    }

    for (auto logger : loggers) {
        logger.get().logAfterPopulateTPG();
    }

    // Evaluate
    auto results =
        this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Save the best score of this generation
    this->updateBestScoreLastGen(results);

    // Remove worst performing roots
    decimateWorstRoots(results);
    // Update the best
    this->updateEvaluationRecords(results);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {
        std::multimap<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*> validationResults;

        if(generationNumber % params.stepValidation == 0 || generationNumber == params.nbGenerations - 1){
            validationResults = evaluateAllRoots(generationNumber, Learn::LearningMode::VALIDATION);
        }
        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }
    }

    for (auto logger : loggers) {
        logger.get().logEndOfTraining();
    }
}



std::shared_ptr<Learn::EvaluationResult> Learn::MujocoMapEliteLearningAgent::evaluateJob(
            TPG::TPGExecutionEngine& tee, const Job& job,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const
{

    if(mapEliteArchive.getDimensions().first == 0){
        return Learn::LearningAgent::evaluateJob(tee, job, generationNumber, mode, le);
    }
    if(dynamic_cast<MujocoWrapper*>(&le) == nullptr){
        throw std::runtime_error("MapElites only support mujoco environments for now");
    }
    MujocoWrapper* mujocoLE = dynamic_cast<MujocoWrapper*>(&le);

    // Only consider the first root of jobs
    const TPG::TPGVertex* root = job.getRoot();

    // Skip the root evaluation process if enough evaluations were already
    // performed. In the evaluation mode only.
    std::shared_ptr<Learn::EvaluationResult> previousEval;
    if (mode == LearningMode::TRAINING &&
        this->isRootEvalSkipped(*root, previousEval)) {
        return previousEval;
    }

    // Init results
    double result = 0.0;
    double utility = 0.0;

    // Number of evaluations
    uint64_t nbEvaluation = (mode == LearningMode::TRAINING) ? this->params.nbIterationsPerPolicyEvaluation:this->params.nbIterationsPerPolicyValidation;

    // Vector of size of descriptors and initialized to 0
    std::vector<double> descriptors = std::vector<double>(mujocoLE->getDescriptors().size(), 0.0);

    // Evaluate nbIteration times
    for (size_t iterationNumber = 0; iterationNumber < nbEvaluation; iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        
        // Same seed at each generation for map elites
        uint64_t hash = hasher(10000) ^ hasher(iterationNumber);

        // Reset the learning Environment
        mujocoLE->reset(hash, mode, iterationNumber, generationNumber);

        uint64_t nbActions = 0;
        while (!mujocoLE->isTerminal() &&
               nbActions < this->params.maxNbActionsPerEval) {
            // Get the actions
            std::vector<double> actionsID =
                tee.executeFromRoot(*root, mujocoLE->getInitActions()).second;
            // Do it
            mujocoLE->doActions(actionsID);

            // Count actions
            nbActions++;
        }

        // Update results
        result += mujocoLE->getScore();
        utility += mujocoLE->getUtility();

        if(mapEliteArchive.getDimensions().first > 0){

            // Get feet contact information
            auto currentDescriptor = mujocoLE->getDescriptors();
            for(size_t i = 0; i< currentDescriptor.size(); i++){
                descriptors[i] += currentDescriptor[i] / nbActions;
            }
        }
    }

    // Get the average value
    for(size_t i = 0; i< descriptors.size() && mapEliteArchive.getDimensions().first > 0; i++){
        descriptors[i] /= nbEvaluation;
    } 

    // Create the EvaluationResult
    auto evaluationResult =
        std::shared_ptr<MapElitesEvaluationResult>(new MapElitesEvaluationResult(
            result / (double)nbEvaluation,
            nbEvaluation,
            utility / (double)nbEvaluation,
            descriptors));

    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }
    return evaluationResult;
}

void Learn::MujocoMapEliteLearningAgent::decimateWorstRoots(
    std::multimap<std::shared_ptr<EvaluationResult>,
                    const TPG::TPGVertex*>& results)
{



    if(mapEliteArchive.getDimensions().first == 0){
        Learn::LearningAgent::decimateWorstRoots(results);
        return;
    }


    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
        preservedRoots;


    while(results.size() > 0){


        // Get the evaluation (casted) and root
        std::shared_ptr<EvaluationResult> eval = results.begin()->first;
        if(dynamic_cast<MapElitesEvaluationResult*>(eval.get()) == nullptr){
            throw std::runtime_error("Evalresult should be castable in mapElites eval results");
        }
        MapElitesEvaluationResult* castEval = dynamic_cast<MapElitesEvaluationResult*>(eval.get());
        const TPG::TPGVertex* root = results.begin()->second;

        // Get the saved evaluation and root
        const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& pairSaved = mapEliteArchive.getArchiveFromDescriptors(castEval->getDescriptors());
        
        
        // The value saved in the archive is better than the current root
        // There is also a verification that the root is not the same

        if(pairSaved.second != nullptr && pairSaved.second != root && pairSaved.first->getResult() > castEval->getResult()){


            // Delete the root
            tpg->removeVertex(*root);
            // Removed stored result (if any)
            this->resultsPerRoot.erase(root);


        // The current root is better than the values saved
        } else if (pairSaved.second != root) {

            // Save the new root and delete the old one
            tpg->removeVertex(*pairSaved.second);

            // Removed stored result (if any)
            this->resultsPerRoot.erase(pairSaved.second);


            if(pairSaved.second != nullptr){
                // Erase it from preserved roots too if the value is in it
                auto it = preservedRoots.find(pairSaved.first);
                if(it != preservedRoots.end() && it->second == pairSaved.second){
                    preservedRoots.erase(it);
                }
            }

            // Saving
            this->mapEliteArchive.setArchiveFromDescriptors(root, eval, castEval->getDescriptors());
            
            // Preserved the root
            preservedRoots.insert(*results.begin()); // Root are presevered if they are override in this same generation !

        }
        results.erase(results.begin());

    }

    
    // Restore root actions
    results.insert(preservedRoots.begin(), preservedRoots.end());
}


void Learn::MujocoMapEliteLearningAgent::updateEvaluationRecords(
            const std::multimap<std::shared_ptr<EvaluationResult>,
                                const TPG::TPGVertex*>& results)
{
    { // Update resultsPerRoot
        for (auto result : results) {
            auto mapIterator = this->resultsPerRoot.find(result.second);
            if (mapIterator == this->resultsPerRoot.end()) {
                // First time this root is evaluated
                this->resultsPerRoot.emplace(result.second, result.first);
            }
            else if (result.first != mapIterator->second) {
                // This root has already been evaluated.
                // If the received result pointer is different from the one
                // stored in the map, update the one in the map by replacing it
                // with the new one (which was combined with the pre-existing
                // one in evalRoot)
                mapIterator->second = result.first;
                // If the received result is associated to the current bestRoot,
                // update it.
                if (result.second == this->bestRoot.first) {
                    this->bestRoot.second = result.first;
                }
            }
        }
    }
    if(results.size()> 0){ // Update bestRoot
        auto iterator = --results.end();
        const std::shared_ptr<EvaluationResult> evaluation = iterator->first;
        const TPG::TPGVertex* candidate = iterator->second;
        // Test the three replacement cases
        // from the simpler to the most complex to test
        if (this->bestRoot.first == nullptr         // NULL case
            || *this->bestRoot.second < *evaluation // new high-score case
            || !this->tpg->hasVertex(
                   *this->bestRoot.first) // bestRoot disappearance
        ) {
            // Replace the best root
            this->bestRoot = {candidate, evaluation};
        }

        // Otherwise do nothing
    }
}



const MapElitesArchive& Learn::MujocoMapEliteLearningAgent::getMapElitesArchive()
{
    return mapEliteArchive;
}