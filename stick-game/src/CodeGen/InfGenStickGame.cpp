#include <iostream>
#include <cfloat>

#include <gegelati.h>

extern "C" {
#include "stick_game.h"
}

#include "../Learn/stickGameAdversarial.h"

/// instantiate global variable used to communicate between the TPG and the environment
int *in1;
int *in2;

int main() {

    /// Import instruction set used during training(required only for gegelati Inference)
    Instructions::Set set;
    auto minus = [](int a, int b) -> double { return (double) a - (double) b; };
    auto cast = [](int a) -> double { return (double) a; };
    auto add = [](double a, double b) -> double { return a + b; };
    auto max = [](double a, double b) -> double { return std::max(a, b); };
    auto nulltest = [](double a) -> double { return (a == 0.0) ? 10.0 : 0.0; };
    auto modulo = [](double a, double b) -> double {
        if (b != 0.0) { return fmod(a, b); }
        else { return DBL_MIN; }
    };

    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo)));
    set.add(*(new Instructions::LambdaInstruction<int, int>(minus)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
    set.add(*(new Instructions::LambdaInstruction<int>(cast)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));

    /// initialise AdversarialLearningEnvironment
    auto le = StickGameAdversarial(false);

    /// Instantiate an Environment and import (required only for gegelati Inference)

    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
    Environment env(set, le.getDataSources(), params.nbRegisters);
    auto tpg = TPG::TPGGraph(env);
    TPG::TPGExecutionEngine tee(env);
    File::TPGGraphDotImporter dotImporter(ROOT_DIR"/src/CodeGen/StickGame_out_best.dot", env, tpg);
    dotImporter.importGraph();
    auto root2 = tpg.getRootVertices().back();

    /// fetch data in the environment
    auto &st = le.getDataSources().at(0).get();
    in1 = st.getDataAt(typeid(int), 0).getSharedPointer<int>().get();
    auto &st2 = le.getDataSources().at(1).get();
    in2 = st2.getDataAt(typeid(int), 0).getSharedPointer<int>().get();

    /// set the number of game
    uint64_t action;
    int playerNb = 0;
    size_t nbParties = 1;
    std::cout << "Game : " << le.toString() << std::endl;
    // let's play, the only way to leave this loop is to enter -1
    while (nbParties != 0) {
        // gets the action the TPG would decide in this situation
        /// to use gegelati Inference uncomment the following line
        action=((const TPG::TPGAction *) tee.executeFromRoot(* root2).back())->getActionID();

        /// to use inference with generated C files uncomment the 2 following lines
//        action = executeFromVertex(root);
//        reset();

        std::cout << "TPG : " << action+1 << std::endl;
        le.doAction(action);
        playerNb = !playerNb;
        // prints the game board
        std::cout << "Game : " << le.toString() << std::endl;

        if (le.isTerminal()) {
            std::cout << "TPG nb" << playerNb << " won !" << std::endl;
            std::cout << "Reseting game..." << std::endl;
            le.reset();
            nbParties--;
            continue;
        }
    }
}