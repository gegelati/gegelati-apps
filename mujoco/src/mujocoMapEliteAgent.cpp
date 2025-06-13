
#include "mujocoMapEliteAgent.h"
#include "mapEliteEvalRes.h"
#include "mujocoEnvironment/mujocoAntWrapper.h"


std::shared_ptr<Learn::EvaluationResult> Learn::MujocoMapEliteLearningAgent::evaluateJob(
            TPG::TPGExecutionEngine& tee, const Job& job,
            uint64_t generationNumber, LearningMode mode,
            LearningEnvironment& le) const
{

    if(dynamic_cast<MujocoAntWrapper*>(&le) == nullptr){
        if(sizeArchive > 0){
            throw std::runtime_error("MapElites only support ant environment for now");

        } else {
            return Learn::LearningAgent::evaluateJob(tee, job, generationNumber, mode, le);
        }
    }
    MujocoAntWrapper* antLE = dynamic_cast<MujocoAntWrapper*>(&le);

    // Only consider the first root of jobs as we are not in adversarial mode
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

    // Number of evaluations
    uint64_t nbEvaluation = (mode == LearningMode::TRAINING) ? this->params.nbIterationsPerPolicyEvaluation:this->params.nbIterationsPerPolicyValidation;

    std::vector<double> all_feet_contact = {0.0, 0.0, 0.0, 0.0};
    std::vector<std::vector<double>> feets;
    std::vector<uint64_t> act;

    // Evaluate nbIteration times
    for (size_t iterationNumber = 0; iterationNumber < nbEvaluation; iterationNumber++) {
        // Compute a Hash
        Data::Hash<uint64_t> hasher;
        uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber);

        // Reset the learning Environment
        antLE->reset(hash, mode, iterationNumber, generationNumber);

        uint64_t nbActions = 0;
        while (!antLE->isTerminal() &&
               nbActions < this->params.maxNbActionsPerEval) {
            // Get the actions
            std::vector<double> actionsID =
                tee.executeFromRoot(*root, antLE->getInitActions()).second;
            // Do it
            antLE->doActions(actionsID);
            
            // Count actions
            nbActions++;
        }

        // Update results
        result += antLE->getScore();

        if(sizeArchive > 0){

            // Get feet contact information
            auto feetContact = antLE->getNbFeetContact();
            for(size_t i = 0; i< feetContact.size(); i++){
                all_feet_contact[i] += feetContact[i] / nbActions;
            }
        }
    }

    for(size_t i = 0; i< all_feet_contact.size() && sizeArchive > 0; i++){
        all_feet_contact[i] /= nbEvaluation;
    } 

    // Create the EvaluationResult
    auto evaluationResult =
        std::shared_ptr<MapElitesEvaluationResult>(new MapElitesEvaluationResult(
            result / (double)nbEvaluation,
            nbEvaluation,
            all_feet_contact));

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



    if(sizeArchive == 0){
        Learn::LearningAgent::decimateWorstRoots(results);
        return;
    }

    std::multimap<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>
        preservedRoots;


    while(results.size() > 0){

        //std::cout<<1<<std::endl;

        // Get the evaluation (casted) and root
        std::shared_ptr<EvaluationResult> eval = results.begin()->first;
        if(dynamic_cast<MapElitesEvaluationResult*>(eval.get()) == nullptr){
            throw std::runtime_error("Evalresult should be castable in mapElites eval results");
        }
        MapElitesEvaluationResult* castEval = dynamic_cast<MapElitesEvaluationResult*>(eval.get());
        const TPG::TPGVertex* root = results.begin()->second;

        //std::cout<<2<<std::endl;
        // Get the saved evaluation and root
        const std::pair<std::shared_ptr<EvaluationResult>, const TPG::TPGVertex*>& pairSaved = getArchiveFromDescriptors(castEval->getFeetContact());
        
        //std::cout<<3<<std::endl;
        
        // The value saved in the archive is better than the current root
        // There is also a verification that the root is not the same

        if(pairSaved.second != nullptr && pairSaved.second != root && pairSaved.first->getResult() > castEval->getResult()){

            //std::cout<<4<<std::endl;
            // Delete the root
            tpg->removeVertex(*root);
            // Removed stored result (if any)
            this->resultsPerRoot.erase(root);


        // The current root is better than the values saved
        } else if (pairSaved.second != root) {
            //std::cout<<5<<std::endl;
            // Save the new root and delete the old one
            tpg->removeVertex(*pairSaved.second);
            //std::cout<<51<<std::endl;
            // Removed stored result (if any)
            this->resultsPerRoot.erase(pairSaved.second);


            //std::cout<<52<<pairSaved.second<<std::endl;
            if(pairSaved.second != nullptr){
                // Erase it from preserved roots too if the value is in it
                auto it = preservedRoots.find(pairSaved.first);
                if(it != preservedRoots.end() && it->second == pairSaved.second){
                    preservedRoots.erase(it);
                }
            }

            //std::cout<<6<<std::endl;
            // Saving
            this->setArchiveFromDescriptors(root, eval, castEval->getFeetContact());
            
            //std::cout<<7<<std::endl;
            // Preserved the root
            preservedRoots.insert(*results.begin()); // Root are presevered if they are override in this same generation !

        }
        results.erase(results.begin());

    }//std::cout<<8<<std::endl;

    
    // Restore root actions
    results.insert(preservedRoots.begin(), preservedRoots.end());

    //std::cout<<"Info "<<results.size()<<" "<<tpg->getNbVertices()<<" "<<preservedRoots.size()<<std::endl;
    
    bool printShit = true;
    if(printShit){
        std::cout<<"\n SUDOKU PRIIIIINT"<<std::endl;
        for(int i = 0; i < 3; ++i) {
                for(int k = 0; k < 3; ++k) {
                    for(int j = 0; j < 3; ++j) {
                        for(int l = 0; l < 3; ++l) {
                            int idx = i*27 + j*9 + k*3 + l;
                            const auto& elem = archive[idx];
                            if(elem.second != nullptr){
                                int val = static_cast<int>(std::round(elem.first->getResult()));
                                std::cout << std::setw(4) << val;
                            } else {
                                std::cout << "   0";  // 4 espaces pour aligner avec setw(4)
                            }
                            if(l != 2) std::cout << ";"; // espace entre colonnes
                        }
                        if(j != 2) std::cout << " | ";
                    }
                    std::cout << "\n";
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


const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& Learn::MujocoMapEliteLearningAgent::getArchiveAt(size_t i, size_t j, size_t k, size_t l) {
    return archive[i * sizeArchive * sizeArchive * sizeArchive +
                j * sizeArchive * sizeArchive +
                k * sizeArchive +
                l];
}

const std::pair<std::shared_ptr<Learn::EvaluationResult>, const TPG::TPGVertex*>& Learn::MujocoMapEliteLearningAgent::getArchiveFromDescriptors(
    const std::vector<double>& descriptors)
{
    return getArchiveAt(
        static_cast<size_t>(std::trunc(descriptors[0] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[1] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[2] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[3] * sizeArchive))
    );
}

void Learn::MujocoMapEliteLearningAgent::setArchiveAt(const TPG::TPGVertex* vertex, std::shared_ptr<Learn::EvaluationResult> eval, size_t i, size_t j, size_t k, size_t l) {
    archive[i * sizeArchive * sizeArchive * sizeArchive +
        j * sizeArchive * sizeArchive +
        k * sizeArchive +
        l] = std::make_pair(eval, vertex);
} 

void Learn::MujocoMapEliteLearningAgent::setArchiveFromDescriptors(const TPG::TPGVertex* vertex, std::shared_ptr<Learn::EvaluationResult> eval, const std::vector<double>& descriptors)
{

    //std::cout<<descriptors[0]<<descriptors[1]<<descriptors[2]<<descriptors[3]<<std::endl;
    setArchiveAt(
        vertex, eval,
        static_cast<size_t>(std::trunc(descriptors[0] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[1] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[2] * sizeArchive)),
        static_cast<size_t>(std::trunc(descriptors[3] * sizeArchive))
    );
}