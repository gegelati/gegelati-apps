
#include "lexicaseSelector.h"
#include "mujocoEnvironment/mujocoWrapper.h"

void Selector::LexicaseSelector::doSelection(
    std::multimap<std::shared_ptr<Learn::EvaluationResult>,
        const TPG::TPGVertex*>& results, Mutator::RNG& rng)
{

    // Vertices selected to be parents
    std::set<const TPG::TPGVertex*> selectedVertices;

    std::map<const std::vector<size_t>, double> bs;

    for(auto& r : results){
        if(bs.empty()){
            for(auto& v: dynamic_cast<Learn::LexicaseEvaluationResult*>(r.first.get())->getScores()){
                bs[v.first] = v.second;
            }
        }

        for(auto& v: dynamic_cast<Learn::LexicaseEvaluationResult*>(r.first.get())->getScores()){
            if(v.second > bs[v.first]){
                bs[v.first] = v.second;
            }
        }
    }
    std::cout<<"  best ";
    for(auto s: bs){
        std::cout<<s.second<<"  ";
    }

    size_t nbSelectedParents = 50;
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
                if (dynamic_cast<Learn::LexicaseEvaluationResult*>(it->first.get())->getScores().at(*testCase) < bestScore - mad/10){
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
    size_t v = 0;
    size_t w = 0;
    while(it != results.end()){
        if(selectedVertices.find(it->second) == selectedVertices.end()){

            graph->removeVertex(*it->second);
            // Removed stored result (if any)
            this->resultsPerRoot.erase(it->second);
            it = results.erase(it);
            v++;
        } else {
            it++;
            w++;
        }
    }


}

std::vector<std::vector<size_t>> Selector::LexicaseSelector::shuffleTestCases(Mutator::RNG& rng)
{
    std::vector<std::vector<size_t>> shuffledTestCases;
    std::vector<std::vector<size_t>> testCases(this->currentTestCases);
    while(testCases.size() > 0){
        auto it = testCases.begin();
        std::advance(it, rng.getUnsignedInt64(0, testCases.size() - 1));

        shuffledTestCases.push_back(*it);
        testCases.erase(it);
    }
    return shuffledTestCases;
}

void Selector::LexicaseSelector::updateTestCases(Mutator::RNG& rng, Learn::LearningMode mode)
{

    this->currentTestCases.clear();
    std::vector<std::vector<size_t>> allTestCasesCopy(this->allTestCases);
    
    size_t nbCases = allTestCases.size();//(mode == Learn::LearningMode::TRAINING) ? params.nbIterationsPerPolicyEvaluation : params.nbIterationsPerPolicyValidation;

    while(this->currentTestCases.size() < nbCases && allTestCasesCopy.size() > 0){
        auto it = allTestCasesCopy.begin();
        std::advance(it, rng.getUnsignedInt64(0, allTestCasesCopy.size() - 1));

        this->currentTestCases.push_back(*it);
        allTestCasesCopy.erase(it);
    }
}

const std::vector<std::vector<size_t>>& Selector::LexicaseSelector::getCurrentTestCases() const
{
    return this->currentTestCases;
}












/* UTILS PART */

std::map<std::vector<size_t>, double> Learn::LexicaseEvaluationResult::getScores() const
{
    return this->scores;
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
            auto it2 = lexiOther->scores.find(it1->first); // Utilisez `find` pour une recherche efficace

            if (it2 != lexiOther->scores.end()) {
                // Pondération avec normalisation
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
    if(dynamic_cast<Selector::LexicaseSelector*>(selector.get()) == nullptr){
        return Learn::LearningAgent::evaluateJob(tee, job, generationNumber, mode, le);
    }


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

    // Number of evaluations
    uint64_t nbEvaluation = (mode == LearningMode::TRAINING)
                                ? this->params.nbIterationsPerPolicyEvaluation
                                : this->params.nbIterationsPerPolicyValidation;

    auto testCases = dynamic_cast<Selector::LexicaseSelector*>(selector.get())->getCurrentTestCases();

    for(size_t taskNumber = 0; taskNumber < testCases.size(); taskNumber++){

        double rTask = 0;

        // Evaluate nbIteration times
        for (size_t iterationNumber = 0; iterationNumber < nbEvaluation;
            iterationNumber++) {
            // Compute a Hash
            Data::Hash<uint64_t> hasher;
            uint64_t hash = hasher(generationNumber) ^ hasher(iterationNumber) ^ hasher(taskNumber);

            dynamic_cast<MujocoWrapper*>(&le)->setObstacles(testCases.at(taskNumber));

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

            // Update utility if used.
            if (le.isUsingUtility()) {
                utility += le.getUtility();
            }
        }
        
        allScores.insert(std::make_pair(testCases.at(taskNumber), rTask / nbEvaluation));
    }


    // Create the EvaluationResult
    auto evaluationResult = std::shared_ptr<LexicaseEvaluationResult>(
        new LexicaseEvaluationResult(allScores, result / (double)(nbEvaluation * testCases.size()), nbEvaluation,
                             utility / (double)(nbEvaluation*testCases.size())));


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
    if(params.selection.selectionMode == "lexicase"){
        dynamic_cast<Selector::LexicaseSelector*>(selector.get())->updateTestCases(rng, Learn::LearningMode::TRAINING);
    }

    // Evaluate
    auto results =
        this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->selector->doSelection(results, rng);
    // Update the evaluation records
    this->selector->updateEvaluationRecords(results);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {
        // Update the test cases in lexicase selection.
        if(params.selection.selectionMode == "lexicase"){
            dynamic_cast<Selector::LexicaseSelector*>(selector.get())->updateTestCases(rng, Learn::LearningMode::VALIDATION);
        }

        std::multimap<std::shared_ptr<Learn::EvaluationResult>,
                      const TPG::TPGVertex*>
            validationResults;

        if (generationNumber % params.stepValidation == 0 ||
            generationNumber == params.nbGenerations - 1) {
            validationResults = evaluateAllRoots(
                generationNumber, Learn::LearningMode::VALIDATION);
        }
        for (auto logger : loggers) {
            logger.get().logAfterValidate(validationResults);
        }
    }

    if (doPopulate) {
        // Populate Sequentially
        Mutator::TPGMutator::populateTPG(
            *this->tpg, *this->selector, this->archive, this->params.mutation,
            this->rng, this->learningEnvironment.getNbActions(), maxNbThreads);
    }

    for (auto logger : loggers) {
        logger.get().logAfterPopulateTPG();
    }

    for (auto logger : loggers) {
        logger.get().logEndOfTraining();
    }
}