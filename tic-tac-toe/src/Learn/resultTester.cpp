#include <iostream>
#include <unordered_set>
#include <numeric>
#include <string>
#include <cfloat>
#include <inttypes.h>

#include <gegelati.h>
#include "resultTester.h"

#include "TicTacToe.h"

int agentTest() {
    // Create the instruction set for programs
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


    // Instantiate the LearningEnvironment
    auto le = TicTacToe();

    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);

    // Instantiate the environment that will embed the LearningEnvironment
    Environment env(set, params, le.getDataSources());

    // Instantiate the TPGGraph that we will loead
    auto tpg = TPG::TPGGraph(env);

    // Instantiate the tee that will handle the decisions taken by the TPG
    TPG::TPGExecutionEngine tee(env);

    // Create an importer for the best graph and imports it
    File::TPGGraphDotImporter dotImporter("out_best.dot", env, tpg);
    dotImporter.importGraph();

    // takes the first root of the graph, anyway out_best has only 1 root (the best)
    auto root = tpg.getRootVertices().front();


    size_t x = 0;
    std::cout<<"Game :\n"<<le.toString()<<std::endl;
    // let's play, the only way to leave this loop is to enter -1
    while(x!=-1){
        // gets the action the TPG would decide in this situation (the result can only be between 0 and 8 included)
        uint64_t action=((const TPG::TPGAction *) tee.executeFromRoot(* root).first.back())->getActionID();
        std::cout<<"TPG : "<<action<<std::endl;
        le.play(action,0);

        // prints the game board
        std::cout<<"Game :\n"<<le.toString()<<std::endl;

        if(le.isTerminal()){
            std::cout<<"Reseting game..."<<std::endl;
            le.reset();
            continue;
        }

        // gets the action of the player, and checks that he did something legal as he has less discipline than a TPG
        std::cout<<"Player move ? "<<std::endl;
        std::cin>>x;
        std::cout<<x<<" chosen"<<std::endl;
        if(x<0 || x>8){
            continue;
        }
        le.play(x,1);


        // prints the game board
        std::cout<<"Game :\n"<<le.toString()<<std::endl;

        if(le.isTerminal()){
            std::cout<<"Reseting game..."<<std::endl;
            le.reset();
        }
    }


    // cleanup
    for (unsigned int i = 0; i < set.getNbInstructions(); i++) {
        delete (&set.getInstruction(i));
    }

    return 0;
}
