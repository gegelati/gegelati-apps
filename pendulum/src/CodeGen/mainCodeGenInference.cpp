#include <iostream>

extern "C" {
#include "externHeader.h"
#include "pendulum.h"
	/// instantiate global variable used to communicate between the TPG and the environment
	double* in1;
}

#include "../Learn/pendulum.h"
#include "../Learn/instructions.h"

#ifndef NO_CONSOLE_CONTROL
#include "../Learn/render.h"
#endif

int main() {
	/// initialise AdversarialLearningEnvironment
	auto le = Pendulum({ 0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0 });

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

#ifndef NO_CONSOLE_CONTROL
	Render::renderInit();
	Render::renderEnv(&angleDisplay, &torqueDisplay, 0, 0);
#endif

	// Do one inference
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

#ifndef NO_CONSOLE_CONTROL
		Render::renderEnv(&angleDisplay, &torqueDisplay, nbActions, 0);
#endif

#ifdef NO_CONSOLE_CONTROL
		std::cout << "TPG : " << action << " \t";
		std::cout << "angle : " << in1[0] << " | float : " << angleDisplay << "\t";
		std::cout << "torque : " << in1[1] << " | float : " << torqueDisplay << std::endl;
#endif
	}
}