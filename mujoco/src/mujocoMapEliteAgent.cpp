
#include "mujocoMapEliteAgent.h"
#include "mapElitesMutator.h"
#include "mapEliteEvalRes.h"
#include "mujocoEnvironment/mujocoAntWrapper.h"

void Learn::MujocoMapEliteLearningAgent::init(uint64_t seed){
    
    ParallelLearningAgent::init(seed);

    if(dynamic_cast<MujocoWrapper*>(&learningEnvironment) == nullptr){
        throw std::runtime_error("MapElites only support mujoco environments for now");
    }


    if (dynamic_cast<const CVTArchiveParametrization*>(&archiveParams) != nullptr) {
        mapEliteArchive = new CvtMapElitesArchive(*dynamic_cast<const CVTArchiveParametrization*>(&archiveParams), this->rng);
    } else {
        mapEliteArchive = new MapElitesArchive(archiveParams);
    }
}

void Learn::MujocoMapEliteLearningAgent::trainOneGeneration(uint64_t generationNumber)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Populate Sequentially
    if(mapEliteArchive->size() > 0){
        Mutator::MapElitesMutator::populateTPG(
            *this->tpg, this->archive, *this->mapEliteArchive, this->params.mutation, this->rng,
            generationNumber, maxNbThreads, useOnlyCloseAddEdges);
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

    if(mapEliteArchive->getDimensions().first == 0){
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
    std::vector<double> descriptors = std::vector<double>(mujocoLE->getNbDescriptors() * archiveParams.nbDescriptorsAnalysis, 0.0);

    // Evaluate nbIteration times
    for (size_t iterationNumber = 0; iterationNumber < nbEvaluation; iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        
        // Same seed at each generation for map elites
        uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber);
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

        if(mapEliteArchive->getDimensions().first > 0){
                
            auto& currentDescriptor = mujocoLE->getDescriptors();
            for(size_t idx1 = 0; idx1 < mujocoLE->getNbDescriptors(); idx1++){
                

                size_t descriptorIndex = 0;
                if(archiveParams.useAbsMeanDescriptor){
                    
                    double value = 0.0;
                    for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                        value += std::abs(currentDescriptor[idx2][idx1]);
                    }
                    descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] 
                        += value / currentDescriptor.size();
                    descriptorIndex += 1;
                }

                if(archiveParams.useMeanDescriptor){

                    double value = 0.0;
                    for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                        value += currentDescriptor[idx2][idx1];
                    }
                    descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] 
                        += value / currentDescriptor.size();
                    descriptorIndex += 1;
                }

                if(archiveParams.useMedianDescriptor || archiveParams.useQuantileDescriptor || archiveParams.useMinMaxDescriptor){
                    std::vector<double> currentDescriptorValues;
                    for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                        currentDescriptorValues.push_back(currentDescriptor[idx2][idx1]);
                    }
                    std::sort(currentDescriptorValues.begin(), currentDescriptorValues.end());

                    if(archiveParams.useMedianDescriptor){
                        descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] += currentDescriptorValues[currentDescriptorValues.size() / 2];
                        descriptorIndex += 1;
                    }
                    if(archiveParams.useQuantileDescriptor){
                        // 25% quantile
                        descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] += currentDescriptorValues[currentDescriptorValues.size() / 4];
                        descriptorIndex += 1;
                        // 75% quantile
                        descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] += currentDescriptorValues[3 * currentDescriptorValues.size() / 4];
                        descriptorIndex += 1;
                    }
                    if(archiveParams.useMinMaxDescriptor){
                        descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] += currentDescriptorValues.front();
                        descriptorIndex += 1;
                        descriptors[descriptorIndex + archiveParams.nbDescriptorsAnalysis * idx1] += currentDescriptorValues.back();
                        descriptorIndex += 1;
                    }
                }
            }

        }
    }

    // Get the average value
    for(size_t i = 0; i< descriptors.size() && mapEliteArchive->getDimensions().first > 0; i++){
        descriptors[i] /= nbEvaluation;
    } 

    if(archiveParams.descriptorName == "programLines"){
        descriptors = std::vector<double>(mujocoLE->getNbDescriptors() * archiveParams.nbDescriptorsAnalysis, 0.0);
        if(archiveParams.typeProgramDescriptor == "nbInstr" || archiveParams.typeProgramDescriptor == "nbInstrUseful"){
            for(auto edge: root->getOutgoingEdges()){
                size_t actionID = dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass();
                size_t nbLines = edge->getProgramSharedPointer()->getNbLines();
                for(size_t idx = 0; idx < nbLines; idx++){
                    if(archiveParams.typeProgramDescriptor == "nbInstr" || !edge->getProgramSharedPointer()->isIntron(idx)){
                        descriptors.at(actionID)++;
                    }
                }
                descriptors.at(actionID) /= params.mutation.prog.maxProgramSize;
            }
        } else if (archiveParams.typeProgramDescriptor == "nbUniqueInstr" || archiveParams.typeProgramDescriptor == "nbUniqueInstrUseful"){
            for(auto edge: root->getOutgoingEdges()){
                size_t actionID = dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass();
                size_t nbLines = edge->getProgramSharedPointer()->getNbLines();
                std::map<size_t, size_t> instructionUsed;
                for(size_t idx = 0; idx < env.getInstructionSet().getNbInstructions(); idx++){
                    instructionUsed.insert(std::make_pair(idx, 0));
                }
                for(size_t idx = 0; idx < nbLines; idx++){
                    if(archiveParams.typeProgramDescriptor == "nbUniqueInstr" || !edge->getProgramSharedPointer()->isIntron(idx)){
                        auto line = edge->getProgramSharedPointer()->getLine(idx);
                        instructionUsed.at(line.getInstructionIndex())++;
                    }
                }
                for(auto& pair: instructionUsed){
                    if(pair.second > 0){
                        descriptors.at(actionID)++;
                    } 
                }
                descriptors.at(actionID) /= instructionUsed.size();
            }
        } else {
            throw std::runtime_error("typeProgramDescriptor not known");
        }

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

std::vector<double> Learn::MujocoMapEliteLearningAgent::updateDescriptorWithMainValues(std::vector<double>& descriptors)
{
    std::vector<double> mainDescriptors;

    // Sort descriptors values
    std::sort(descriptors.begin(), descriptors.end());

    // Compute mean value
    if(archiveParams.useMainMeanDescriptor || archiveParams.useMainStdDescriptor){
        double meanValue = 0.0;
        for(size_t i = 0; i < descriptors.size(); i++){
            meanValue += descriptors[i];
        }
        meanValue /= descriptors.size();

        if(archiveParams.useMainMeanDescriptor){
            mainDescriptors.push_back(meanValue);
        }

        // Compute std value if it is used
        if(archiveParams.useMainStdDescriptor){
            double stdValue = 0.0;
            for(size_t i = 0; i < descriptors.size(); i++){
                stdValue += (descriptors[i] - meanValue) * (descriptors[i] - meanValue);
            }
            stdValue = std::sqrt(stdValue / descriptors.size());
            mainDescriptors.push_back(stdValue);
        }
    }

    // Compute median value
    if(archiveParams.useMainMedianDescriptor){
        mainDescriptors.push_back(descriptors[descriptors.size() / 2]);
    }

    // Compute max value
    if(archiveParams.useMainMaxDescriptor){
        mainDescriptors.push_back(descriptors.back());
    }   
    // Compute min value
    if(archiveParams.useMainMinDescriptor){
        mainDescriptors.push_back(descriptors.front());
    }
    return mainDescriptors;

}

void Learn::MujocoMapEliteLearningAgent::decimateWorstRoots(
    std::multimap<std::shared_ptr<EvaluationResult>,
                    const TPG::TPGVertex*>& results)
{
    
    for(auto pair: mapEliteArchive->getAllArchive()){
        if(pair.second!=nullptr){
            if(!tpg->hasVertex(*pair.second)){
                std::cout<<"not normal here"<<std::endl;
            }
        }
    }


    if(mapEliteArchive->getDimensions().first == 0){
        Learn::LearningAgent::decimateWorstRoots(results);
        return;
    }

    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
        preservedRoots;


    size_t numberNewValues = 0;

    for(auto it = results.begin(); it != results.end(); it++){
        // The root is already in the archive
        if(this->mapEliteArchive->containsRoot(it->second)){
            // The root has been reevaluated, delete it from the archive if it has not been evaluated enough times
            this->mapEliteArchive->removeRootFromArchiveIfNotComplete(it->second, params.maxNbEvaluationPerPolicy);
        }
    }


    while(results.size() > 0){

        // Utilise l'élément avec le plus grand score (fin du multimap)
        auto it = --results.end();

        // Get the evaluation (casted) and root
        std::shared_ptr<EvaluationResult> eval = it->first;
        if(dynamic_cast<MapElitesEvaluationResult*>(eval.get()) == nullptr){
            throw std::runtime_error("Evalresult should be castable in mapElites eval results");
        }
        MapElitesEvaluationResult* castEval = dynamic_cast<MapElitesEvaluationResult*>(eval.get());
        const TPG::TPGVertex* root = it->second;

        std::vector<double> descriptorUsed(castEval->getDescriptors());
        if(this->archiveParams.nbMainDescriptors > 0){
            descriptorUsed = updateDescriptorWithMainValues(descriptorUsed);
        }

        // Get the saved evaluation and root
        std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*> pairSaved = mapEliteArchive->getArchiveFromDescriptors(descriptorUsed);

        // The value saved in the archive is better than the current root
        // There is also a verification that the root is not the same
        if(pairSaved.second != nullptr && pairSaved.second != root && pairSaved.first->getResult() >= castEval->getResult()){

            // Removed stored result (if any)
            this->resultsPerRoot.erase(root);
            // Delete the root
            tpg->removeVertex(*root);

        // The current root is better than the values saved
        } else if (pairSaved.second != root) {



            // Removed stored result (if any)
            this->resultsPerRoot.erase(pairSaved.second);


            numberNewValues++;

            // Saving
            this->mapEliteArchive->setArchiveFromDescriptors(root, eval, descriptorUsed);

            // Save the new root and delete the old one
            tpg->removeVertex(*pairSaved.second);
            
            // Preserved the root
            preservedRoots.insert(*it); // Root are preserved if they are override in this same generation !
        }
        results.erase(it);
    }

    std::cout<<"  nv "<<numberNewValues<<"  ";

    
    // Restore root actions
    results.insert(preservedRoots.begin(), preservedRoots.end());
    std::cout<<preservedRoots.size()<<" ";

    for(auto pair: mapEliteArchive->getAllArchive()){
        if(pair.second!=nullptr){
            if(!tpg->hasVertex(*pair.second)){
                std::cout<<"not normal here"<<std::endl;
                for(auto v: preservedRoots){
                    if(pair.second == v.second){
                        std::cout<<"root was preserved"<<std::endl;
                    }
                }

            }
        }
    }
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
    return *mapEliteArchive;
}