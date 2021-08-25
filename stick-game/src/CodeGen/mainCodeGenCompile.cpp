#include <iostream>
#include <cfloat>

#include <gegelati.h>

int main(){

    Instructions::Set set;
    auto minus = [](int a, int b)->double {return (double)a - (double)b; };
    auto cast = [](int a)->double {return (double)a; };
    auto add = [](double a, double b)->double {return a + b; };
    auto max = [](double a, double b)->double {return std::max(a, b); };
    auto nulltest = [](double a)->double {return (a == 0.0) ? 10.0 : 0.0; };
    auto modulo = [](double a, double b)->double {
        if (b != 0.0) { return fmod(a, b); }
        else { return  DBL_MIN; }	};

    set.add(*(new Instructions::LambdaInstruction<double, double>(modulo, "$0 = (($2) != 0.0) ? fmod($1, $2) : DBL_MIN ;")));
    set.add(*(new Instructions::LambdaInstruction<int, int>(minus, "$0 = (double)($1) - (double)($2);")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(add, "$0 = $1 + $2;")));
    set.add(*(new Instructions::LambdaInstruction<int>(cast, "$0 = (double)($1);")));
    set.add(*(new Instructions::LambdaInstruction<double, double>(max, "$0 = (($1) < ($2)) ? ($2) : ($1); ")));
    set.add(*(new Instructions::LambdaInstruction<double>(nulltest, "$0 = ($1 == 0.0) ? 10.0 : 0.0;")));

    std::cout << set.getInstruction(3).getNbOperands() << std::endl;
    Data::PrimitiveTypeArray<int> remainingSticks(1);
    Data::PrimitiveTypeArray<int> hints(4);

    std::string path(ROOT_DIR "/src/CodeGen/");

    std::vector<std::reference_wrapper<const Data::DataHandler>> data = {hints, remainingSticks};
    Learn::LearningParameters params;
    File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
    Environment dotEnv(set, data, params.nbRegisters);

    TPG::TPGGraph dotGraph(dotEnv);
    std::string filename(path+"StickGame_out_best.dot");

    File::TPGGraphDotImporter dot(filename.c_str(), dotEnv, dotGraph);
    dot.importGraph();

    CodeGen::TPGGenerationEngine tpggen("stick_game", dotGraph, "src/");
    tpggen.generateTPGGraph();

    return 0;

}

