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
	size_t seed = 2;

	/// fetch data in the environment
	auto dataSources = le.getDataSources();
	auto& st = dataSources.at(0).get();
	auto dataSharedPointer = st.getDataAt(typeid(double), 0).getSharedPointer<double>();
	in1 = dataSharedPointer.get();

	/// Init the pendulum
	le.reset(seed);
	float angleDisplay = (float)(in1[0]);
	float torqueDisplay = (float)(in1[1]);

#ifndef NO_CONSOLE_CONTROL
	Render::renderInit();
	Render::renderEnv(&angleDisplay, &torqueDisplay, 0, 0);
#else
	auto colWidth(7);
	std::cout << std::setprecision(2) << std::fixed << std::right;
	std::cout << std::setw(colWidth) << "frame" << std::setw(colWidth) << "action";
	std::cout << std::setw(colWidth) << "angle" << std::setw(colWidth) << "torque" << std::endl;
#endif

	// Do one inference
	int nbActions = 0;
	uint64_t actions[1000];
	// measure time 
	auto start = std::chrono::system_clock::now();
	while (nbActions < 1000 && !le.isTerminal()) {

		/// inference with generated C files
		actions[nbActions] = inferenceTPG();
		// Do the action 
		le.doAction(actions[nbActions]);

		// Display the result
		angleDisplay = (float)(in1[0]);
		torqueDisplay = (float)(in1[1]);

#ifndef NO_CONSOLE_CONTROL
		Render::renderEnv(&angleDisplay, &torqueDisplay, nbActions, 0);
#else
		std::cout << std::setw(colWidth) << nbActions << std::setw(colWidth) << actions[nbActions];
		std::cout << std::setw(colWidth) << in1[0] << std::setw(colWidth) << in1[1] << std::endl;
#endif
		nbActions++;
	}
	auto stop = std::chrono::system_clock::now();

	// do a replay to subtract non-inference time
	size_t iter = 0;
	le.reset(seed);
	// in1[0] = M_PI; // reset pendulum position
	auto startReplay = std::chrono::system_clock::now();
	while (iter < nbActions) {
		// Do the action 
		le.doAction(actions[iter]);

		// Display the result
		angleDisplay = (float)(in1[0]);
		torqueDisplay = (float)(in1[1]);

#ifndef NO_CONSOLE_CONTROL
		Render::renderEnv(&angleDisplay, &torqueDisplay, nbActions, 0);
#else
		std::cout << std::setw(colWidth) << iter << std::setw(colWidth) << actions[iter];
		std::cout << std::setw(colWidth) << in1[0] << std::setw(colWidth) << in1[1] << std::endl;
#endif
		iter++;
	}
	auto stopReplay = std::chrono::system_clock::now();
	auto totalTime = ((std::chrono::duration<double>)(stop - start)).count();
	auto replayTime = ((std::chrono::duration<double>)(stopReplay - startReplay)).count();
	std::cout << std::setprecision(6) << " Total time: " << totalTime << std::endl;
	std::cout << std::setprecision(6) << "  Env. time: " << replayTime << std::endl;
	std::cout << std::setprecision(6) << "Infer. time: " << totalTime-replayTime << std::endl;
}