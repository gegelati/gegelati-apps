#include <iostream>

extern "C" {
#include "externHeader.h"
#include "pendulum.h"
	/// instantiate global variable used to communicate between the TPG and the environment
	double* in1;
}

#include "../Learn/pendulum.h"
#include "../Learn/render.h"
#include "../Learn/instructions.h"


int main() {
	/// Import instruction set used during training(required only for gegelati Inference)
	Instructions::Set set;
	fillInstructionSet(set);

	/// initialise AdversarialLearningEnvironment
	auto le = Pendulum({ 0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0 });

	/// Instantiate an Environment and import (required only for gegelati Inference)
	Learn::LearningParameters params;
	File::ParametersParser::loadParametersFromJson(ROOT_DIR"/params.json", params);
	Environment env(set, le.getDataSources(), params.nbRegisters, params.nbProgramConstant);

	/// fetch data in the environment
	auto dataSources = le.getDataSources();
	auto& st = dataSources.at(0).get();
	auto dataSharedPointer = st.getDataAt(typeid(double), 0).getSharedPointer<double>();
	in1 = dataSharedPointer.get();

	/// set the number of generation
	uint64_t action;
	in1[0] = M_PI;
	float angleDisplay = (float)(in1[0]);
	float torqueDisplay = (float)(in1[1]);

	// Do one inference
	Render::renderInit();
	Render::renderEnv(&angleDisplay, &torqueDisplay, 0, 0);
	int nbActions = 0;
	while (nbActions < 1000 && !le.isTerminal()) {
		nbActions++;

		/// inference with generated C files
		action = inferenceTPG();

		// Do the action 
		le.doAction(action);

		// Display the result
		angleDisplay = (float)(in1[0]);
		torqueDisplay = (float)(in1[1]);
#ifdef DEBUG
		std::cout << "TPG : " << action << std::endl;
		std::cout << "angle : " << in1[0] << " | float : " << angleDisplay << std::endl;
		std::cout << "couple : " << in1[1] << " | float : " << torqueDisplay << std::endl;
#endif

		Render::renderEnv(&angleDisplay, &torqueDisplay, nbActions, 0);
	}
}