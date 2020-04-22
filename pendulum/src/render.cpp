#include <iostream>

#define _USE_MATH_DEFINES // To get M_PI
#include <math.h>

// SDL libs
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <float.h>

#include <gegelati.h>
#include "pendulum.h"
#include "render.h"

/**
* \brief Structure holding attributes for a display.
*/
typedef struct sdlDisplay {
	SDL_Renderer* renderer;
	SDL_Window* screen;
	SDL_Texture* texture;
	TTF_Font* font;
}sdlDisplay;

// sdlDisplay instance
static sdlDisplay display;

void Render::renderInit() {
	display.screen = NULL;
	display.renderer = NULL;

	// Initialize SDL
	fprintf(stderr, "SDL_Init_Start\n");
	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}
	fprintf(stderr, "SDL_Init_End\n");


	// Initialize SDL TTF for text display
	fprintf(stderr, "SDL_TTF_Init_Start\n");
	if (TTF_Init()) {
		printf("TTF initialization failed: %s\n", TTF_GetError());
	}
	fprintf(stderr, "SDL_TTF_Init_End\n");

	// Open Font for text display
	display.font = TTF_OpenFont(PATH_TTF, 20);
	if (!display.font) {
		printf("TTF_OpenFont: %s\n", TTF_GetError());
	}

	// Create window
	display.screen = SDL_CreateWindow("Environment_Display", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		DISPLAY_W, DISPLAY_H, SDL_WINDOW_SHOWN);
	if (!display.screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
	// Create renderer
	display.renderer = SDL_CreateRenderer(display.screen, -1, SDL_RENDERER_ACCELERATED);
	if (!display.renderer) {
		fprintf(stderr, "SDL: could not create renderer - exiting\n");
		exit(1);
	}
	SDL_Surface* surface = IMG_Load(PATH);

	if (!surface) {
		fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
		exit(1);
	}

	display.texture = SDL_CreateTextureFromSurface(display.renderer,
		surface);
	if (!display.texture) {
		fprintf(stderr, "SDL: could not create texture - exiting\n");
		exit(1);
	}
}

void Render::controlerLoop(std::atomic<bool>& exit, std::atomic<bool>& toggleDisplay, std::atomic<bool>& doDisplay,
	const TPG::TPGVertex** bestRoot, const Instructions::Set& set, Pendulum& pendulumLE, const Learn::LearningParameters& params,
	std::atomic<uint64_t>& generation) {

	// Display
	renderInit();

	exit = false;

	// Prepare objects for replays
	float angleDisplay = M_PI;
	Environment env(set, pendulumLE.getDataSources(), 8);
	TPG::TPGExecutionEngine tee(env);
	auto frame = 0;
	std::deque<std::tuple<uint64_t, double>> replay;

	while (!exit) {

		// Was a replay requested?
		if (doDisplay) {
			// Compute all moves
			replay.clear();
			pendulumLE.reset(0, Learn::LearningMode::VALIDATION);
			for (auto action = 0; action < params.maxNbActionsPerEval; action++) {
				auto vertexList = tee.executeFromRoot(**bestRoot);
				pendulumLE.doAction(((const TPG::TPGAction*)vertexList.back())->getActionID());
				replay.push_back(std::make_tuple(action, pendulumLE.getAngle()));
			}

			doDisplay = false;
		}

		if (!replay.empty()) {
			angleDisplay = (float)std::get<1>(replay.front());
			frame = (float)std::get<0>(replay.front());
			replay.pop_front();
		}

		int event = Render::renderEnv(&angleDisplay, frame, generation);
		switch (event) {
		case 1:
			std::cout << "Display " << ((toggleDisplay) ? "de" : "re") << "activated." << std::endl;
			toggleDisplay = !toggleDisplay;
			break;

		case -1:
			exit = true;
			doDisplay = false;
			break;
		case 0:
		default:
			// Nothing to do
			break;
		}
	}
	Render::renderFinalize();
	printf("\nProgram will terminate at the end of next generation.\n");
	std::cout.flush();
}

void Render::displayText(const char* text, int posX, int posY) {
	// Color of text
	SDL_Color colorGreen = { 0, 255, 0, 255 };
	SDL_Color colorWhite = { 255, 255, 255, 255 };

	SDL_Surface* textSurf = TTF_RenderText_Blended(display.font, text,
		colorGreen);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(display.renderer,
		textSurf);

	int width, height;
	SDL_QueryTexture(texture, NULL, NULL, &width, &height);
	SDL_Rect textRect;

	textRect.x = posX;
	textRect.y = posY;
	textRect.w = width;
	textRect.h = height;
	SDL_RenderCopy(display.renderer, texture, NULL, &textRect);

	/* Free resources */
	SDL_FreeSurface(textSurf);
	SDL_DestroyTexture(texture);
}

int Render::renderEnv(float* state, uint64_t frame, uint64_t generation) {
	static long int i = 0;
	static double max_fps = 0.;
	static double avg_fps = 0.;
	static double min_fps = DBL_MAX;

	// Select the color for drawing. It is set to red here. 
	SDL_SetRenderDrawColor(display.renderer, 255, 255, 255, 255);
	// Clear the entire screen to our selected color.
	SDL_RenderClear(display.renderer);

	// Position of the pendulum in the window
	SDL_Rect dest = { 225, 250, 49, 234 };
	SDL_Point center = { 25, 15 };

	// Convert the angle to degree with the offset to match the python training
	float angle = 180.f - state[0] * 180.f / ((float)M_PI);

	// Display the pendulum
	SDL_RenderCopyEx(display.renderer, display.texture, NULL, &dest, angle, &center, SDL_FLIP_NONE);

	// Print Generation text
	char generationString[100];
	sprintf(generationString, "   gen: %4lld", generation);
	Render::displayText(generationString, 0, 0);

	// Print FrameNumber text
	char frameNumber[17];
	sprintf(frameNumber, "frame: %4lld", frame);
	Render::displayText(frameNumber, 0, 22);

	// Proceed to the actual display
	SDL_RenderPresent(display.renderer);

	// Smoother rendering
	SDL_Delay(25);

	SDL_Event event;
	// Grab next events off the queue.
	SDL_PollEvent(&event);
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_t:
			return 1;
		case SDLK_q:
			return -1;
		}
	case SDL_QUIT:
		return -1;
	default:
		break;
	}

	return 0;
}

void Render::renderFinalize()
{
	SDL_DestroyTexture(display.texture);
	SDL_DestroyRenderer(display.renderer);
	SDL_DestroyWindow(display.screen);
}
