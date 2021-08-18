#include <iostream>

extern "C" {
#include "pendulum.h"
}

#include "../Learn/pendulum.h"
#include "../Learn/render.h"

/// instantiate global variable used to communicate between the TPG and the environment
double *in1;

int main() {
    /// Import instruction set used during training(required only for gegelati Inference)
    Instructions::Set set;
    auto minus = [](double a, double b) -> double { return a - b; };
    auto add = [](double a, double b) -> double { return a + b; };
    auto mult = [](double a, double b) -> double { return a * b; };
    auto div = [](double a, double b) -> double { return a / b; };
    auto max = [](double a, double b) -> double { return std::max(a, b); };
    auto ln = [](double a) -> double { return std::log(a); };
    auto exp = [](double a) -> double { return std::exp(a); };
    auto cos = [](double a) -> double { return std::cos(a); };
    auto sin = [](double a) -> double { return std::sin(a); };
    auto tan = [](double a) -> double { return std::tan(a); };
    auto pi = [](double a) -> double { return M_PI; };
    auto multByConst = [](double a, Data::Constant c) -> double { return a * (double) c / 10.0; };

    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(mult)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(div)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double>(exp)));
    set.add(*(new Instructions::LambdaInstruction<double>(ln)));
    set.add(*(new Instructions::LambdaInstruction<double>(cos)));
    set.add(*(new Instructions::LambdaInstruction<double>(sin)));
    set.add(*(new Instructions::LambdaInstruction<double>(tan)));
    set.add(*(new Instructions::LambdaInstruction<double, Data::Constant>(multByConst)));
    set.add(*(new Instructions::LambdaInstruction<double>(pi)));
    /// initialise AdversarialLearningEnvironment
    // ex stickGame : auto le = StickGameAdversarial();
    auto le = Pendulum({0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0});

    /// Instantiate an Environment and import (required only for gegelati Inference)

    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
    Environment env(set, le.getDataSources(), params.nbRegisters, params.nbProgramConstant);

    auto tpg = TPG::TPGGraph(env);
    TPG::TPGExecutionEngine tee(env);
    File::TPGGraphDotImporter dotImporter(ROOT_DIR"/src/CodeGen/Pendulum_out_best.dot", env, tpg);
    dotImporter.importGraph();
    auto root2 = tpg.getRootVertices().front();

    /// fetch data in the environment
    auto &st = le.getDataSources().at(0).get();
    in1 = st.getDataAt(typeid(double), 0).getSharedPointer<double>().get();

    /// set the number of game
    size_t nbGame = 1;
    uint64_t action;
    int playerNb = 0;
    in1[0] = M_PI;
    float angleDisplay = (float) (in1[0]);
    float torqueDisplay = (float) (in1[1]);
    //    std::cout<<"Game :\n"<<le.toString()<<std::endl;
    // let's play, the only way to leave this loop is finish all game
    Render::renderInit();
    Render::renderEnv(&angleDisplay, &torqueDisplay, 0, nbGame);
    int i = 1;
    while (nbGame != 0) {
        // gets the action the TPG would decide in this situation
        /// to use gegelati Inference uncomment the following line
//        action = ((const TPG::TPGAction *) tee.executeFromRoot(*root2).back())->getActionID();

        /// to use inference with generated C files uncomment the 2 following lines
        action = executeFromVertex(root);
        reset();

        std::cout << "TPG : " << action << std::endl;
        playerNb = !playerNb;
        // prints the game board
//        std::cout<<"Game :\n"<<le.toString()<<std::endl;
        angleDisplay = (float) (in1[0]);
        torqueDisplay = (float) (in1[1]);
#ifdef DEBUG
        std::cout << "angle : " << in1[0] << " | float : " << angleDisplay << std::endl;
        std::cout << "couple : " << in1[1] << " | float : " << torqueDisplay << std::endl;
#endif
        le.doAction(action);

        Render::renderEnv(&angleDisplay, &torqueDisplay, i, nbGame);

        if (le.isTerminal()) {
            std::cout << "TPG nb" << playerNb << " won !" << std::endl;
            std::cout << "Reseting game..." << std::endl;
            le.reset();
            nbGame--;
            continue;
        }
        i++;
    }
}