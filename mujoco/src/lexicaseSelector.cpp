#include <fstream>
#include <sstream>
#include "lexicaseSelector.h"
#include "mujocoEnvironment/mujocoWrapper.h"

void Selector::LexicaseSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
        const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{
    // Vertices selected to be parents
    std::set<const TPG::TPGVertex*> selectedVertices;


    size_t nbSelectedParents = results.size() * params.selection.lexicase.ratioSelectedRoots;
    for(size_t idx = 0; idx < nbSelectedParents; idx++){

        // Shuffle the test cases
        std::vector<std::vector<size_t>> shuffledTestCases = this->shuffleTestCases(rng);

        // Copy the results;
        auto currentResult(results);

        while(currentResult.size() > 1 && shuffledTestCases.size() > 0){

            auto testCase = shuffledTestCases.begin();

            double bestScore = -std::numeric_limits<double>::infinity();
            auto it = currentResult.begin();

            // Get best score and erase the already selected vertices
            std::vector<double> scores;
            while(it != currentResult.end()){
                if(selectedVertices.find(it->second) != selectedVertices.end()){
                    it = currentResult.erase(it);
                } else {
                    auto* lexResult = dynamic_cast<Learn::LexicaseEvaluationResult*>(it->first.get());
                    scores.push_back(lexResult->getScores().at(*testCase));
                    if (scores.back() > bestScore){
                        bestScore = dynamic_cast<Learn::LexicaseEvaluationResult*>(it->first.get())->getScores().at(*testCase);
                    }
                    it++;
                }
            }

            // We want to compute the median absolute deviation of the population
            // Start with the median
            std::sort(scores.begin(), scores.end());
            double median = scores[scores.size() / 2];
            if (scores.size() % 2 == 0) {
                median = (median + scores[scores.size() / 2 - 1]) / 2.0;
            }

            // Compute the absolute deviation
            std::vector<double> absoluteDeviations;
            for (double score : scores) {
                absoluteDeviations.push_back(std::abs(score - median));
            }

            // Compute the median of the absolute deviation
            std::sort(absoluteDeviations.begin(), absoluteDeviations.end());
            double mad = absoluteDeviations[absoluteDeviations.size() / 2];
            if (absoluteDeviations.size() % 2 == 0) {
                mad = (mad + absoluteDeviations[absoluteDeviations.size() / 2 - 1]) / 2.0;
            }

            // Get best score and erase the already selected vertices
            it = currentResult.begin();
            while(it != currentResult.end()){
                if (dynamic_cast<Learn::LexicaseEvaluationResult*>(it->first.get())->getScores().at(*testCase) < bestScore - mad * params.selection.lexicase.alpha){
                    it = currentResult.erase(it);
                } else {
                    it++;
                }
            }
            shuffledTestCases.erase(testCase);
        }

        if(currentResult.size() == 1){
            selectedVertices.insert(currentResult.begin()->second);
        } else {
            auto it = currentResult.begin();
            std::advance(it, rng.getUnsignedInt64(0, currentResult.size() - 1));
            selectedVertices.insert(it->second);
        }
    }
    


    auto it = results.begin();
    while(it != results.end()){
        if(selectedVertices.find(it->second) == selectedVertices.end()){

            graph->removeVertex(*it->second);
            // Removed stored result (if any)
            this->resultsPerRoot.erase(it->second);
            it = results.erase(it);
        } else {
            it++;
        }
    }
}

std::vector<std::vector<size_t>> Selector::LexicaseSelector::shuffleTestCases(Mutator::RNG& rng) const
{
    std::vector<std::vector<size_t>> shuffledTestCases;
    std::vector<std::vector<size_t>> currentTestCases(this->testCases);
    while(currentTestCases.size() > 0){
        auto it = currentTestCases.begin();
        std::advance(it, rng.getUnsignedInt64(0, currentTestCases.size() - 1));

        shuffledTestCases.push_back(*it);
        currentTestCases.erase(it);
    }
    return shuffledTestCases;
}

/*void Selector::LexicaseSelector::updateTestCases(Mutator::RNG& rng, Learn::LearningMode mode)
{

    this->currentTestCases.clear();
    std::vector<std::vector<size_t>> allTestCasesCopy(this->allTestCases);
    
    if(mode == Learn::LearningMode::TRAINING){
        while(this->currentTestCases.size() < allTestCases.size() && allTestCasesCopy.size() > 0){
            auto it = allTestCasesCopy.begin();
            std::advance(it, rng.getUnsignedInt64(0, allTestCasesCopy.size() - 1));

            this->currentTestCases.push_back(*it);
            allTestCasesCopy.erase(it);
        }
    } else {
        std::set<size_t> allUniqueCases;
        for(auto& vect: this->allTestCases){
            allUniqueCases.insert(vect.begin(), vect.end());
        }

        std::vector<size_t> vectorAllCases(allUniqueCases.begin(), allUniqueCases.end());
        this->currentTestCases.push_back(vectorAllCases);
        //this->currentTestCases = {{0}, {1}, {2}, {3}, {4}, {0,1,2,3,4}};
    }
}*/

const std::vector<std::vector<size_t>>& Selector::LexicaseSelector::getCurrentTestCases() const
{
    return this->testCases;
}









/* HIERARCHICAL PART */

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

        

        
        std::cout<<std::endl;
        std::cout<<"Number of action roots "<< graph->getRootActions().size()<< "And number of roots" << graph->getNbRootVertices()<<std::endl;

        // Sélection tournoi spécifique à chaque testCase
        for (auto& actionGroup : resultsActionPerTestCase) {
            if (!actionGroup.empty()) {
                TournamentSelector::doSelection(actionGroup, rng);
                std::cout<<"Number of action roots "<< graph->getRootActions().size()<< "And number of roots" << graph->getNbRootVertices()<< " and number of vertices to delete " << this->getVerticesToDelete().size()<<std::endl;;
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

        std::vector<size_t> v2 = {0,0,0,0,0};
        for(const TPG::TPGAction* action: graph->getRootActions()){
            v2.at(action->getActionID()) += 1;
        }
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


/* UTILS PART */

std::map<std::vector<size_t>, double> Learn::LexicaseEvaluationResult::getScores() const
{
    return this->scores;
}


std::map<std::vector<size_t>, double> Learn::LexicaseEvaluationResult::getSuccess() const
{
    return this->success;
}

Learn::EvaluationResult& Learn::LexicaseEvaluationResult::operator+=(
    const Learn::EvaluationResult& other)
{
    // Type Check (Must be done in all override)
    // This test will succeed in child class.
    const std::type_info& thisType = typeid(*this);
    if (typeid(other) != thisType) {
        throw std::runtime_error("Type mismatch between LexicaseEvaluationResults.");
    }

    // If the added type is Learn::EvaluationResult
    if (thisType == typeid(Learn::LexicaseEvaluationResult)) {

        auto* lexiOther = dynamic_cast<const Learn::LexicaseEvaluationResult*>(&other);

        // Weighted addition of results
        this->result = this->result * (double)this->nbEvaluation +
                       lexiOther->result * (double)lexiOther->nbEvaluation;
        this->result /= (double)this->nbEvaluation + (double)lexiOther->nbEvaluation;

        // Weighted addition of utility
        this->utility = this->utility * (double)this->nbEvaluation +
                        lexiOther->utility * (double)lexiOther->nbEvaluation;
        this->utility /=
            (double)this->nbEvaluation + (double)lexiOther->nbEvaluation;

        // Addition ot nbEvaluation
        this->nbEvaluation += lexiOther->nbEvaluation;

        auto it1 = this->scores.begin();
        while (it1 != this->scores.end()) {
            auto it2 = lexiOther->scores.find(it1->first);

            if (it2 != lexiOther->scores.end()) {
                it1->second = (it1->second * (double)this->nbEvaluation +
                            it2->second * (double)lexiOther->nbEvaluation) /
                            (double)(this->nbEvaluation + lexiOther->nbEvaluation);
            }
            it1++;
        }

        it1 = this->success.begin();
        while (it1 != this->success.end()) {
            auto it2 = lexiOther->success.find(it1->first);

            if (it2 != lexiOther->success.end()) {
                it1->second = (it1->second * (double)this->nbEvaluation +
                            it2->second * (double)lexiOther->nbEvaluation) /
                            (double)(this->nbEvaluation + lexiOther->nbEvaluation);
            }
            it1++;
        }
    }

    return *this;
}

std::shared_ptr<Learn::EvaluationResult> Learn::LexicaseAgent::evaluateJob(
    TPG::TPGExecutionEngine& tee, const Job& job,
    uint64_t generationNumber, LearningMode mode,
    LearningEnvironment& le) const
{
    /*if(dynamic_cast<Selector::LexicaseSelector*>(selector.get()) == nullptr){
        return Learn::LearningAgent::evaluateJob(tee, job, generationNumber, mode, le);
    }*/


    // Only consider the first root of jobs as we are not in adversarial mode
    const TPG::TPGVertex* root = job.getRoot();

    std::shared_ptr<Learn::EvaluationResult> previousEval;
    if (mode == LearningMode::TRAINING &&
        this->isRootEvalSkipped(*root, previousEval)) {
        return previousEval;
    }

    // Init results
    double result = 0.0;

    // Init utility
    double utility = 0.0;

    std::map<std::vector<size_t>, double> allScores;
    std::map<std::vector<size_t>, double> allSuccess;

    // Number of evaluations
    uint64_t nbEvaluation = (mode == LearningMode::TRAINING)
                                ? this->params.nbIterationsPerPolicyEvaluation
                                : this->params.nbIterationsPerPolicyValidation;

    std::vector<std::vector<size_t>> currentTestCases;
    size_t nbTestCase = 1;
    if(mode == Learn::LearningMode::TRAINING){
        currentTestCases = trainingTestCases;
        if(dynamic_cast<Selector::LexicaseSelector*>(selector.get()) != nullptr){

            if(params.selection._isHierarchical && dynamic_cast<const TPG::TPGAction*>(root) != nullptr){
                currentTestCases.clear();
                currentTestCases.push_back(trainingTestCases.at(dynamic_cast<const TPG::TPGAction*>(root)->getActionID()));

            } else if (params.selection._isHierarchical && params.selection._selectionMode == "tournament"){

                // concat currentTestCases without duplicates
                std::set<size_t> v;
                for(auto& vec: currentTestCases){
                    v.insert(vec.begin(), vec.end());
                }
                currentTestCases.clear();
                currentTestCases.push_back(std::vector<size_t>(v.begin(), v.end()));
            }
        }
        nbTestCase = currentTestCases.size();
    }
    else {
        currentTestCases = validationTestCases;
        nbTestCase = currentTestCases.size();
    }
    if(nbTestCase == 0) nbTestCase++;

    // print if team is action or team, if mode is training or validation and the number of test cases
    std::cout << "Evaluating " << (dynamic_cast<const TPG::TPGAction*>(root) ? "action" : "team") << " in " << (mode == Learn::LearningMode::TRAINING ? "training" : "validation") << " mode with " << nbTestCase << " test cases." << std::endl;
    if(dynamic_cast<const TPG::TPGAction*>(root) != nullptr){
        std::cout<<"evaluating an action of ID "<<dynamic_cast<const TPG::TPGAction*>(root)->getActionID()<<std::endl;
    }

    for(size_t taskNumber = 0; taskNumber < nbTestCase; taskNumber++){


        double rTask = 0;
        double successTask = 0;

        // Evaluate nbIteration times
        for (size_t iterationNumber = 0; iterationNumber < nbEvaluation;
            iterationNumber++) {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber) ^ hasher(taskNumber);

            if(currentTestCases.size() > 1){
                if(dynamic_cast<Selector::LexicaseSelector*>(selector.get()) == nullptr && mode == Learn::LearningMode::TRAINING){
                    throw std::runtime_error("Cannot use more than one type of task at the same time without lexicase selection");
                }
                dynamic_cast<MujocoWrapper*>(&le)->setObstacles(currentTestCases.at(taskNumber));
            } else if (currentTestCases.size() == 1) {
                std::vector<size_t> nonConstTestCase = currentTestCases[0];
                dynamic_cast<MujocoWrapper*>(&le)->setObstacles(nonConstTestCase);
            } else {
                std::vector<size_t> v;
                dynamic_cast<MujocoWrapper*>(&le)->setObstacles(v);
            }
            // Reset the learning Environment
            le.reset(hash, mode, iterationNumber, generationNumber);


            uint64_t nbActions = 0;
            while (!le.isTerminal() &&
                nbActions < this->params.maxNbActionsPerEval) {
                // Get the actions
                std::vector<double> actionsID =
                    tee.executeFromRoot(*root, le.getInitActions()).second;
                // Do it
                le.doActions(actionsID);
                // Count actions
                nbActions++;
            }


            // Update results
            result += le.getScore();
            rTask += le.getScore();
            successTask += dynamic_cast<MujocoWrapper*>(&le)->getNbSuccess();

            // Update utility if used.
            if (le.isUsingUtility()) {
                utility += le.getUtility();
            }
        }
        
        
        if(dynamic_cast<Selector::LexicaseSelector*>(selector.get()) != nullptr || mode != Learn::LearningMode::TRAINING){
            allScores.insert(std::make_pair(currentTestCases.at(taskNumber), rTask / nbEvaluation));
            allSuccess.insert(std::make_pair(currentTestCases.at(taskNumber), successTask / nbEvaluation));

        }
    }


    // Create the EvaluationResult
    auto evaluationResult = std::shared_ptr<LexicaseEvaluationResult>(
        new LexicaseEvaluationResult(allScores, allSuccess, result / (double)(nbEvaluation * nbTestCase), nbEvaluation,
                             utility / (double)(nbEvaluation*nbTestCase)));


    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }

    return evaluationResult;
}

void Learn::LexicaseAgent::trainOneGeneration(uint64_t generationNumber,
                                              bool doPopulate)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Update the test cases in lexicase selection.
    if(params.selection._selectionMode == "lexicase" || params.selection._isHierarchical){
        //dynamic_cast<Selector::LexicaseSelector*>(selector.get())->updateTestCases(rng, Learn::LearningMode::TRAINING);
    }

    // Evaluate
    auto results =
        this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->selector->launchSelection(results, rng);
    // Update the evaluation records
    this->selector->updateEvaluationRecords(results);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {
        // Update the test cases in lexicase selection.
        if(params.selection._selectionMode == "lexicase" || params.selection._isHierarchical){
            //dynamic_cast<Selector::LexicaseSelector*>(selector.get())->updateTestCases(rng, Learn::LearningMode::VALIDATION);
        }

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      const TPG::TPGVertex*>
            validationResults;

        if (generationNumber % params.stepValidation == 0 ||
            generationNumber == params.nbGenerations - 1) {
            validationResults = evaluateAllRoots(
                generationNumber, Learn::LearningMode::VALIDATION);


            
        }

        this->lastValidationResults = validationResults;

        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }

        
    }

    if (doPopulate) {
        // Populate Sequentially
        Mutator::TPGMutator::populateTPG(
            *this->tpg, *this->selector, this->archive, this->params.mutation,
            this->rng, this->trainingTestCases.size(), maxNbThreads);
    }

    for (auto logger : loggers) {
        logger.get().logAfterPopulateTPG();
    }

    for (auto logger : loggers) {
        logger.get().logEndOfTraining();
    }
}

void Learn::LexicaseAgent::init(uint64_t seed)
{
    LearningAgent::init(seed);

    // Update the ids of the actions in hierarchical selection
    if(params.selection._isHierarchical && params.mutation.tpg.ratioTeamsOverActions != 0 && params.mutation.tpg.ratioTeamsOverActions != 1){

        size_t nbCases = trainingTestCases.size();
        std::vector<const TPG::TPGAction *> rootActions(tpg->getRootActions());
        size_t idx = 0;
        for(const TPG::TPGAction* action: rootActions){
            tpg->changeActionID(*action, idx % nbCases);
            idx++;
        }
    } else if (params.selection._isHierarchical){
        throw std::runtime_error("Hierarchical selection requires a mix of teams and actions (ratioTeamsOverActions != 0 and != 1)");
    }
}