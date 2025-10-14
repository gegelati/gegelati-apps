
#include "multiLearningAgent.h"

std::shared_ptr<Learn::EvaluationResult> Learn::MultiLearningAgent::evaluateJob(
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

    /*std::ostringstream prinfInfo;
    // print if team is action or team, if mode is training or validation and the number of test cases
    prinfInfo << "Evaluating " << (dynamic_cast<const TPG::TPGAction*>(root) ? "action" : "team") << " in " << (mode == Learn::LearningMode::TRAINING ? "training" : "validation") << " mode with " << nbTestCase << " test cases." << std::endl;
    if(dynamic_cast<const TPG::TPGAction*>(root) != nullptr){
        prinfInfo<<"evaluating an action of ID "<<dynamic_cast<const TPG::TPGAction*>(root)->getActionID()<<std::endl;
    }
    prinfInfo<<"ARchive of edges: ";
    auto edges = root->getOutgoingEdges();
    for(auto edge: edges){
        prinfInfo<<edge->getProgramSharedPointer()->getArchive()<<" - ";
    }prinfInfo<<std::endl;

    std::cout<<prinfInfo.str()<<std::endl;*/


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
    auto evaluationResult = std::shared_ptr<MultiEvaluationResult>(
        new MultiEvaluationResult(allScores, allSuccess, result / (double)(nbEvaluation * nbTestCase), nbEvaluation,
                             utility / (double)(nbEvaluation*nbTestCase)));


    // Combine it with previous one if any
    if (previousEval != nullptr) {
        *evaluationResult += *previousEval;
    }

    return evaluationResult;
}

void Learn::MultiLearningAgent::trainOneGeneration(uint64_t generationNumber,
                                              bool doPopulate)
{
    for (auto logger : loggers) {
        logger.get().logNewGeneration(generationNumber);
    }

    // Evaluate
    auto results =
        this->evaluateAllRoots(generationNumber, LearningMode::TRAINING);
    for (auto logger : loggers) {
        logger.get().logAfterEvaluate(results);
    }

    // Remove worst performing roots
    this->selector->launchSelection(results, this->rng);
    // Update the evaluation records
    this->selector->updateEvaluationRecords(results);

    for (auto logger : loggers) {
        logger.get().logAfterDecimate();
    }

    // Does a validation or not according to the parameter doValidation
    if (params.doValidation) {

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
            *this->tpg, *this->selector, this->params.mutation,
            this->rng, this->trainingTestCases.size(), maxNbThreads);
    }

    for (auto logger : loggers) {
        logger.get().logAfterPopulateTPG();
    }

    for (auto logger : loggers) {
        logger.get().logEndOfTraining();
    }
}

void Learn::MultiLearningAgent::init(uint64_t seed)
{
    // Initialize Randomness
    this->rng.setSeed(seed);

    // Initialize the tpg
    Mutator::TPGMutator::initRandomTPG(
        *this->tpg, params.mutation, this->rng,
        this->learningEnvironment.getNbActions());


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

    this->resetProgramArchive();

    // Populate Sequentially
    Mutator::TPGMutator::populateTPG(
        *this->tpg, *this->selector, this->params.mutation,
        this->rng, this->learningEnvironment.getNbActions(), maxNbThreads);

    // Clear the archives
    this->teamArchive.clear();
    for(Archive* archive: this->actionArchives){
        archive->clear();
    }

    // Clear the best agent in the selector
    this->selector->forgetPreviousResults();

}