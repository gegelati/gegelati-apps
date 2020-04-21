#ifndef RENDER_H
#define RENDER_H
/*
* The code of the renderer is adapted from Florian Arrestier's code released
* under CECILL - C License.
* Link : https://github.com/preesm/preesm-apps/tree/master/org.ietr.preesm.reinforcement_learning
*/

#include <thread>
#include <atomic>

#define DISPLAY_W 500
#define DISPLAY_H 500

#define PATH PROJECT_ROOT_PATH "/dat/pendulum.png"
#define PATH_TTF PROJECT_ROOT_PATH "/dat/DejaVuSans.ttf"

namespace Render {
	void controlerLoop(std::atomic<bool>& exit, std::atomic<bool>& toggleDisplay, std::atomic<bool>& doDisplay,
		const TPG::TPGVertex** bestRoot, const Instructions::Set& set, Pendulum& pendulumLE, const Learn::LearningParameters& params,
		std::atomic<uint64_t>& generation);

	/**
	 * @brief Initializes SDL renderer and TTF font
	 */
	void renderInit(void);

	/**
	 * @brief Display the environment
	 *
	 * @param size
	 * @param state Angular state of the system
	 * \return
	 * - -1: Exit
	 * -  0: No event
	 * -  1: Toggle display
	 */
	int renderEnv(float* state, uint64_t action, uint64_t generation);

	/**
	 * @brief Close SDL and destroy opened textures
	 */
	void renderFinalize(void);
}

#endif //RENDER_H
