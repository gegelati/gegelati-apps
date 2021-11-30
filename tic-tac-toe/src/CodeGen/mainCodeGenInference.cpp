#include <iostream>
#include <string>
#include <cfloat>
extern "C" {
#include "TicTacToe.h"
}

#include "../Learn/TicTacToe.h"
/// instantiate global variable used to communicate between the TPG and the environment
double* in1;

int main(){
    /// Import instruction set used during training(required only for gegelati Inference)
    Instructions::Set set;
    auto minus = [](double a, double b)->double {return (double)a - (double)b; };
    auto add = [](double a, double b)->double {return a + b; };
    auto max = [](double a, double b)->double {return std::max(a, b); };
    auto modulo = [](double a, double b)->double {return b != 0.0 ? fmod(a,b):DBL_MIN;};
    auto nulltest = [](double a)->double {return (a == -1.0) ? 10.0 : 0.0; };
    auto circletest = [](double a)->double {return (a == 0.0) ? 10.0 : 0.0; };
    auto crosstest = [](double a)->double {return (a == 1.0) ? 10.0 : 0.0; };
    auto test15 = [](double a)->double {return (a >= 15.0) ? 10.0 : 0.0; };
    auto cond = [](double a, double b) -> double { return a < b ? -a : a; };


    set.add(*(new Instructions::LambdaInstruction<double, double>(minus)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max)));
    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo)));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest)));
    set.add(*(new Instructions::LambdaInstruction<double>(circletest)));
    set.add(*(new Instructions::LambdaInstruction<double>(crosstest)));
    set.add(*(new Instructions::LambdaInstruction<double>(test15)));
    set.add(*(new Instructions::LambdaInstruction<double,double>(cond)));

    /// initialise AdversarialLearningEnvironment
    auto le = TicTacToe();

    /// Instantiate an Environment and import (required only for gegelati Inference)

    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
    Environment env(set, le.getDataSources(), params.nbRegisters);

    auto tpg = TPG::TPGGraph(env);
    TPG::TPGExecutionEngine tee(env);
    File::TPGGraphDotImporter dotImporter(ROOT_DIR"/src/CodeGen/TicTacToe_out_best.dot", env, tpg);
    dotImporter.importGraph();
    auto root2 = tpg.getRootVertices().front();

    /// fetch data in the environment
    auto& st = le.getDataSources().at(0).get();
    in1 = st.getDataAt(typeid(double), 0).getSharedPointer<double>().get();

    /// set the number of game
    size_t nbParties = 1;
    uint64_t action;
    int playerSymbol = 0;
    // let's play, the only way to leave this loop is finish all games
    while(nbParties!=0){
        /// to use inference with generated C files uncomment the 2 following lines
        action = inferenceTPG();

        std::cout<<"TPG : "<<action<<std::endl;
        le.play(action,playerSymbol);
        playerSymbol = (!playerSymbol);

        // prints the game board
        std::cout<<"Game :\n"<<le.toString()<<std::endl;

        if(le.isTerminal()){
            std::cout<<"Resetting game..."<<std::endl;
            le.reset();
            --nbParties;
            continue;
        }

    }

    return 0;
}