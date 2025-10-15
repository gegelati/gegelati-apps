
#include "mujocoMapEliteAgent.h"
#include "mapElitesMutator.h"
#include "mapEliteEvalRes.h"
#include "mujocoEnvironment/mujocoAntWrapper.h"

void Learn::MujocoMapEliteLearningAgent::init(uint64_t seed){
    
    ParallelLearningAgent::init(seed);

    if(dynamic_cast<MujocoWrapper*>(&learningEnvironment) == nullptr){
        throw std::runtime_error("MapElites only support mujoco environments for now");
    }

    for(auto& pair: this->archiveParams){
        if (dynamic_cast<const CVTArchiveParametrization*>(pair.second) != nullptr) {
            this->mapEliteArchives.insert(std::make_pair(pair.first, (new CvtMapElitesArchive(*dynamic_cast<const CVTArchiveParametrization*>(pair.second), this->rng))));
        } else {
            this->mapEliteArchives.insert(std::make_pair(pair.first, (new MapElitesArchive(*pair.second))));
        }
    }

}

void Learn::MujocoMapEliteLearningAgent::trainOneGeneration(uint64_t generationNumber)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Populate Sequentially
    if(mapEliteArchives.size() > 0){
        Mutator::MapElitesMutator::populateTPG(
            *this->tpg, this->archive, this->mapEliteArchives, this->params.mutation, this->rng,
            generationNumber, maxNbThreads);
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

    if(mapEliteArchives.size() == 0){
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
    std::map<Descriptor::DescriptorType, std::vector<double>> mapDescriptors;
    for(auto pairDescriptor: archiveParams){
        mapDescriptors.insert(std::make_pair(pairDescriptor.first, std::vector<double>(pairDescriptor.second->nbDescriptorsAnalysis * mujocoLE->getNbDescriptors().at(pairDescriptor.first), 0.0)));
    }

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

        for(auto& pairDescriptor: mapDescriptors){
            if(Descriptor::isBehavioralDescriptor(pairDescriptor.first)){
                updateBehavioralDescriptors(
                    pairDescriptor.second, 
                    archiveParams.at(pairDescriptor.first),
                    mujocoLE->getNbDescriptors().at(pairDescriptor.first),
                    mujocoLE->getDescriptors().at(pairDescriptor.first)
                );
            }
        }
    }

    for(auto& pairDescriptor: mapDescriptors){
        if(Descriptor::isGenotypicDescriptor(pairDescriptor.first)){
            updateGenotypicDescriptors(
                pairDescriptor.second, 
                archiveParams.at(pairDescriptor.first),
                root
            );
        } else {
            for(size_t idx = 0; idx < pairDescriptor.second.size(); idx++){
                // Get the average value
                pairDescriptor.second.at(idx) /= nbEvaluation;
            }
        }
    }


    // Create the EvaluationResult
    auto evaluationResult =
        std::shared_ptr<MapElitesEvaluationResult>(new MapElitesEvaluationResult(
            result / (double)nbEvaluation,
            nbEvaluation,
            utility / (double)nbEvaluation,
            mapDescriptors));

    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }
    return evaluationResult;
}

void Learn::MujocoMapEliteLearningAgent::updateGenotypicDescriptors(
    std::vector<double>& descriptors, const ArchiveParametrization* localArchiveParam, 
    const TPG::TPGVertex* root) const
{
    if(localArchiveParam->descriptorType == Descriptor::DescriptorType::NbInstr || localArchiveParam->descriptorType == Descriptor::DescriptorType::NbInstrUseful){
        for(auto edge: root->getOutgoingEdges()){
            size_t actionID = dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass();
            size_t nbLines = edge->getProgramSharedPointer()->getNbLines();
            for(size_t idx = 0; idx < nbLines; idx++){
                if(localArchiveParam->descriptorType == Descriptor::DescriptorType::NbInstr || !edge->getProgramSharedPointer()->isIntron(idx)){
                    descriptors.at(actionID)++;
                }
            }
            descriptors.at(actionID) /= params.mutation.prog.maxProgramSize;
        }
    } else if (localArchiveParam->descriptorType == Descriptor::DescriptorType::NbUniqueInstr || localArchiveParam->descriptorType == Descriptor::DescriptorType::NbUniqueInstrUseful){
        for(auto edge: root->getOutgoingEdges()){
            size_t actionID = dynamic_cast<TPG::TPGActionEdge*>(edge)->getActionClass();
            size_t nbLines = edge->getProgramSharedPointer()->getNbLines();
            std::map<size_t, size_t> instructionUsed;
            for(size_t idx = 0; idx < env.getInstructionSet().getNbInstructions(); idx++){
                instructionUsed.insert(std::make_pair(idx, 0));
            }
            for(size_t idx = 0; idx < nbLines; idx++){
                if(localArchiveParam->descriptorType == Descriptor::DescriptorType::NbUniqueInstr || !edge->getProgramSharedPointer()->isIntron(idx)){
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

void Learn::MujocoMapEliteLearningAgent::updateBehavioralDescriptors(
    std::vector<double>& descriptors, const ArchiveParametrization* localArchiveParam, 
    size_t nbDescriptors, const std::vector<std::vector<double>> environmentDescriptors) const
{
                
    auto& currentDescriptor = environmentDescriptors;
    for(size_t idx1 = 0; idx1 < nbDescriptors; idx1++){
        size_t descriptorIndex = 0;
        if(localArchiveParam->useAbsMeanDescriptor){
            
            double value = 0.0;
            for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                value += std::abs(currentDescriptor[idx2][idx1]);
            }
            descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] 
                += value / currentDescriptor.size();
            descriptorIndex += 1;
        }

        if(localArchiveParam->useMeanDescriptor){

            double value = 0.0;
            for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                value += currentDescriptor[idx2][idx1];
            }
            descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] 
                += value / currentDescriptor.size();
            descriptorIndex += 1;
        }

        if(localArchiveParam->useMedianDescriptor || localArchiveParam->useQuantileDescriptor || localArchiveParam->useMinMaxDescriptor){
            std::vector<double> currentDescriptorValues;
            for(size_t idx2 = 0; idx2 < currentDescriptor.size(); idx2++){
                currentDescriptorValues.push_back(currentDescriptor[idx2][idx1]);
            }
            std::sort(currentDescriptorValues.begin(), currentDescriptorValues.end());

            if(localArchiveParam->useMedianDescriptor){
                descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] += currentDescriptorValues[currentDescriptorValues.size() / 2];
                descriptorIndex += 1;
            }
            if(localArchiveParam->useQuantileDescriptor){
                // 25% quantile
                descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] += currentDescriptorValues[currentDescriptorValues.size() / 4];
                descriptorIndex += 1;
                // 75% quantile
                descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] += currentDescriptorValues[3 * currentDescriptorValues.size() / 4];
                descriptorIndex += 1;
            }
            if(localArchiveParam->useMinMaxDescriptor){
                descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] += currentDescriptorValues.front();
                descriptorIndex += 1;
                descriptors[descriptorIndex + localArchiveParam->nbDescriptorsAnalysis * idx1] += currentDescriptorValues.back();
                descriptorIndex += 1;
            }
        }
    }
}

std::vector<double> Learn::MujocoMapEliteLearningAgent::updateDescriptorWithMainValues(std::vector<double>& descriptors, Descriptor::DescriptorType descriptorType)
{
    std::vector<double> mainDescriptors;

    // Sort descriptors values
    std::sort(descriptors.begin(), descriptors.end());

    // Compute mean value
    if(archiveParams.at(descriptorType)->useMainMeanDescriptor || archiveParams.at(descriptorType)->useMainStdDescriptor){
        double meanValue = 0.0;
        for(size_t i = 0; i < descriptors.size(); i++){
            meanValue += descriptors[i];
        }
        meanValue /= descriptors.size();

        if(archiveParams.at(descriptorType)->useMainMeanDescriptor){
            mainDescriptors.push_back(meanValue);
        }

        // Compute std value if it is used
        if(archiveParams.at(descriptorType)->useMainStdDescriptor){
            double stdValue = 0.0;
            for(size_t i = 0; i < descriptors.size(); i++){
                stdValue += (descriptors[i] - meanValue) * (descriptors[i] - meanValue);
            }
            stdValue = std::sqrt(stdValue / descriptors.size());
            mainDescriptors.push_back(stdValue);
        }
    }

    // Compute median value
    if(archiveParams.at(descriptorType)->useMainMedianDescriptor){
        mainDescriptors.push_back(descriptors[descriptors.size() / 2]);
    }

    // Compute max value
    if(archiveParams.at(descriptorType)->useMainMaxDescriptor){
        mainDescriptors.push_back(descriptors.back());
    }   
    // Compute min value
    if(archiveParams.at(descriptorType)->useMainMinDescriptor){
        mainDescriptors.push_back(descriptors.front());
    }
    return mainDescriptors;

}

void Learn::MujocoMapEliteLearningAgent::decimateWorstRoots(
    std::multimap<std::shared_ptr<EvaluationResult>,
                    const TPG::TPGVertex*>& results)
{

    if(mapEliteArchives.size() == 0){
        Learn::LearningAgent::decimateWorstRoots(results);
        return;
    }

    // Clear values reevaluated
    for(auto& pair: this->mapEliteArchives){
        MapElitesArchive* mapEliteArchive = pair.second;
        for(auto it = results.begin(); it != results.end(); it++){
            // The root is already in the archive
            if(mapEliteArchive->containsRoot(it->second)){
                // The root has been reevaluated, delete it from the archive if it has not been evaluated enough times
                mapEliteArchive->removeRootFromArchiveIfNotComplete(it->second, params.maxNbEvaluationPerPolicy);
            }
        }
    }

    for(auto& pair: this->mapEliteArchives){
        const Descriptor::DescriptorType& descriptorType = pair.first;
        MapElitesArchive* mapEliteArchive = pair.second;

        std::vector<const TPG::TPGVertex*> verticesToDelete;

        size_t numberNewValues = 0;

        for (auto it = results.rbegin(); it != results.rend(); ++it){


            // Get the evaluation (casted) and root
            std::shared_ptr<EvaluationResult> eval = it->first;
            if(dynamic_cast<MapElitesEvaluationResult*>(eval.get()) == nullptr){
                throw std::runtime_error("Evalresult should be castable in mapElites eval results");
            }
            MapElitesEvaluationResult* castEval = dynamic_cast<MapElitesEvaluationResult*>(eval.get());
            const TPG::TPGVertex* root = it->second;

            std::vector<double> descriptorUsed(castEval->getMapDescriptors().at(descriptorType));
            if(this->archiveParams.at(descriptorType)->nbMainDescriptors > 0){
                descriptorUsed = updateDescriptorWithMainValues(descriptorUsed, descriptorType);
            }

            // Get the saved evaluation and root
            std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*> pairSaved = mapEliteArchive->getArchiveFromDescriptors(descriptorUsed);

            // The value saved in the archive is better than the current root
            // There is also a verification that the root is not the same
            if(pairSaved.second != nullptr && pairSaved.second != root && pairSaved.first->getResult() >= castEval->getResult()){
                // Nothing happened

            // The current root is better than the values saved
            } else if (pairSaved.second != root) {
                numberNewValues++;

                // Saving
                mapEliteArchive->setArchiveFromDescriptors(root, eval, descriptorUsed);
            }
        }

        std::cout<<"  nv "<<numberNewValues<<"  ";
    }

    for (auto it = results.begin(); it != results.end(); ) {
        bool containRoot = false;
        for(auto& pairArchive: this->mapEliteArchives){
            if(pairArchive.second->containsRoot(it->second)){
                containRoot = true;
                break;
            }
        }

        if (!containRoot) {
            this->resultsPerRoot.erase(it->second);
            tpg->removeVertex(*it->second);
            it = results.erase(it); // erase returns next iterator
        } else {
            ++it;
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



const MapElitesArchive& Learn::MujocoMapEliteLearningAgent::getMapElitesArchiveAt(Descriptor::DescriptorType descriptor)
{
    return *mapEliteArchives.at(descriptor);
}