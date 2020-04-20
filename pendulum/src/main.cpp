#include <iostream>

#include "pendulum.h"

extern "C" {
#include "render.h"
}

int main() {
	std::cout << "Hello Pendulum" << std::endl;

	renderInit();

	Pendulum p({ 0.05, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0 });
	p.reset(0);


	for (auto idx = 0; idx < 200; idx++) {
		p.doAction(0);

		printf("%3d %1.2f %c%1.2f\n", idx, p.getAngle(), (p.getVelocity() > 0) ? ' ' : '-', fabs(p.getVelocity()));
		float currAngle = p.getAngle();
		renderEnv(0, &currAngle);
	}

	renderFinalize();
}