#include <iostream>
#include <numeric>
#include <thread>
#include <atomic>
#include <chrono>
#include <inttypes.h>

#define _USE_MATH_DEFINES // To get M_PI

#include <math.h>
#include <iostream>
#include <file/parametersParser.h>
#include "instructions/set.h"
#include "tpg/tpgGraph.h"
#include "environment.h"
#include "file/tpgGraphDotExporter.h"
#include "instructions/lambdaInstruction.h"
#include "code_gen/ProgramGenerationEngine.h"
#include "code_gen/TpgGenerationEngine.h"
#include "file/tpgGraphDotImporter.h"
#include "float.h"


int main() {

    std::cout << "Start Pendulum application." << std::endl;

    // Create the instruction set for programs
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

    set.add(*(new Instructions::LambdaInstruction<double, double>(minus, "$0 = $1 - $2;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add, "$0 = $1 + $2;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(mult, "$0 = $1 * $2;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(div, "$0 = $1 / $2;")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max, "$0 = (($1) < ($2)) ? ($2) : ($1);")));
    set.add(*(new Instructions::LambdaInstruction<double>(exp, "$0 = exp($1);")));
    set.add(*(new Instructions::LambdaInstruction<double>(ln, "$0 = log($1);")));
    set.add(*(new Instructions::LambdaInstruction<double>(cos, "$0 = cos($1);")));
    set.add(*(new Instructions::LambdaInstruction<double>(sin, "$0 = sin($1);")));
    set.add(*(new Instructions::LambdaInstruction<double>(tan, "$0 = tan($1);")));
    set.add(*(new Instructions::LambdaInstruction<double, Data::Constant>(multByConst,
                                                                          "$0 = $1 * ((double)($2) / 10.0);")));
    set.add(*(new Instructions::LambdaInstruction<double>(pi, "$0 = M_PI;")));

    std::string path(ROOT_DIR "/src/CodeGen/");

    Data::PrimitiveTypeArray<double> currentState{2};
    std::vector<std::reference_wrapper<const Data::DataHandler>> data = {currentState};

    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(
            ROOT_DIR"/params.json", params);
    Environment dotEnv(set, data, params.nbRegisters, params.nbProgramConstant);
    TPG::TPGGraph dotGraph(dotEnv);

    std::string filename(path + "Pendulum_out_best.dot");
    File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
    dot.importGraph();

    CodeGen::TPGGenerationEngine tpggen("pendulum", dotGraph, "src/");
    tpggen.generateTPGGraph();


    return 0;
}