#include <fstream>
#include <sstream>
#include "lexicaseSelector.h"

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
                    auto* lexResult = dynamic_cast<Learn::MultiEvaluationResult*>(it->first.get());
                    scores.push_back(lexResult->getScores().at(*testCase));
                    if (scores.back() > bestScore){
                        bestScore = dynamic_cast<Learn::MultiEvaluationResult*>(it->first.get())->getScores().at(*testCase);
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
                if (dynamic_cast<Learn::MultiEvaluationResult*>(it->first.get())->getScores().at(*testCase) < bestScore - mad * params.selection.lexicase.alpha){
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










