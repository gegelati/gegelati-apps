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
/**
* Namespace for rendering pendulum with the SDL2 lib.
*/
namespace Render {
	/**
	* \brief Controller loop for the display of replays.
	*
	* Function initialazing and controlling a display rendering replays of the 
	* pendulum oscillations.
	* This function should be launched in its own thread.
	*
	* \param[in,out] exit Boolean value for controlling the end of the display 
	* and the main program.
	* \param[in,out] toggleDisplay Boolean value for activating and disabling
	* the creation of replays for the best policy of the main learning loop, 
	* at the end of each generation.
	* \param[in,out] doDisplay Boolean value used as a semaphore to control the 
	* creation of a new replay. This boolean value is set to true to start the 
	* creation of a new replay, and this function resets it to false to 
	* indicate that the replay has been created.
	* \param[in] bestRoot TPGVertex from which the replay will be created.
	* \param[in] set Instructions::Set of the TPGGraph to which the bestRoot 
	* belongs.
	* \param[in,out] pendulumLE Pendulum whose replays are created.
	* \param[in] params LearningParameters of the main loop (needed for replays 
	* creation).
	* \param[in] generation Integer value representing the current generation 
	* of the training process.
	* 
	*/
	void controllerLoop(std::atomic<bool>& exit, std::atomic<bool>& toggleDisplay, std::atomic<bool>& doDisplay,
		const TPG::TPGVertex** bestRoot, const Instructions::Set& set, Pendulum& pendulumLE, const Learn::LearningParameters& params,
		std::atomic<uint64_t>& generation);

	/**
	 * \brief Initializes SDL renderer and TTF font
	 */
	void renderInit();

	/**
	* \brief Function for rendering text in the displayed window.
	*
	* \param[in] text The string of character to display.
	* \param[in] posX Abscissa of the text top-left corner.
	* \param[in] posY Ordinate of the text top-left corner.
	*/
	void displayText(const char* text, int posX, int posY);

	/**
	 * \brief Display the environment.
	 *
	 * \param[in] state Current angle of the pendulum to display.
	 * \return An integer value is returned to the controller loop depending 
	 * on the action made by the user:
	 * - -1: Exit the program.
	 * -  0: No event
	 * -  1: Toggle display of replay after each generation.
	 */
	int renderEnv(float* state, uint64_t action, uint64_t generation);

	/**
	 * \brief Close SDL and destroy opened textures.
	 */
	void renderFinalize();
}

#endif //RENDER_H
