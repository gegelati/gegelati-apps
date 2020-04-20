
/*
* The code of the renderer is adapted from Florian Arrestier's code released
* under CECILL - C License.
* Link : https://github.com/preesm/preesm-apps/tree/master/org.ietr.preesm.reinforcement_learning
*/

#ifndef RENDER_H
#define RENDER_H

#define DISPLAY_W 500
#define DISPLAY_H 500

#define PATH PROJECT_ROOT_PATH "/dat/pendulum.png"
#define PATH_TTF PROJECT_ROOT_PATH "/dat/DejaVuSans.ttf"

/**
 * @brief Initializes SDL renderer and TTF font
 */
void renderInit(void);

/**
 * @brief Display the environment
 *
 * @param size
 * @param state Angular state of the system
 */
void renderEnv(int size, float* state);

/**
 * @brief Close SDL and destroy opened textures
 */
void renderFinalize(void);

#endif //RENDER_H
