#include <iostream>
#include <fstream>
#include <file/parametersParser.h>
#include "instructions/set.h"
#include "tpg/tpgGraph.h"
#include "environment.h"
#include "file/tpgGraphDotExporter.h"
#include "instructions/lambdaInstruction.h"
#include "codeGen/ProgramGenerationEngine.h"
#include "codeGen/TpgGenerationEngine.h"
#include "file/tpgGraphDotImporter.h"
#include "float.h"
#include <codeGen/tpgGenerationEngineFactory.h>

int main(){

    Instructions::Set set;
    auto minus = [](double a, double b) -> double { return (double)a - (double)b; };
    auto add = [](double a, double b) -> double { return a + b; };
    auto max = [](double a, double b) -> double { return std::max(a, b); };
    auto modulo = [](double a, double b) -> double { return b != 0.0 ? fmod(a, b) : DBL_MIN; };
    auto nulltest = [](double a) -> double { return (a == -1.0) ? 10.0 : 0.0; };
    auto circletest = [](double a) -> double { return (a == 0.0) ? 10.0 : 0.0; };
    auto crosstest = [](double a) -> double { return (a == 1.0) ? 10.0 : 0.0; };
    auto test15 = [](double a) -> double { return (a >= 15.0) ? 10.0 : 0.0; };
    auto cond = [](double a, double b) -> double { return a < b ? -a : a; };


    set.add(*(new Instructions::LambdaInstruction<double, double>(minus,"$0 = (double)($1) - (double)($2);")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add,"$0 = $1 + $2;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max,"$0 = (($1) < ($2)) ? ($2) : ($1); ")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo,"$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;")));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest,"$0 = ($1) == -1.0 ? 10.0 : 0.0;")));
    set.add(*(new Instructions::LambdaInstruction<double>(circletest,"$0 = ($1) == 0.0 ? 10.0 : 0.0;")));
    set.add(*(new Instructions::LambdaInstruction<double>(crosstest,"$0 = ($1) == 1.0 ? 10.0 : 0.0;")));
    set.add(*(new Instructions::LambdaInstruction<double>(test15,"$0 = ($1) >= 15.0 ? 10.0 : 0.0;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(cond,"$0 = ($1) < ($2) ? -1*($1) : ($1);")));

    std::string path(ROOT_DIR "/src/CodeGen/");

    Data::PrimitiveTypeArray<double> currentState(9);

    std::vector<std::reference_wrapper<const Data::DataHandler>> data = {currentState};
    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR "/params.json", params);
    Environment dotEnv(set, params, data);

    TPG::TPGGraph dotGraph(dotEnv);
    std::string filename(path+"TicTacToe_out_best.dot");
    File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
    dot.importGraph();

    CodeGen::TPGGenerationEngineFactory factory(CodeGen::TPGGenerationEngineFactory::switchMode);
    std::unique_ptr<CodeGen::TPGGenerationEngine> tpggen = factory.create("TicTacToe", dotGraph, "src/");
    tpggen->generateTPGGraph();

    return 0;

}

