//
// Created by thomas on 19/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_RENDER_H
#define SFML_FLAPPY_BIRD_RENDER_H

#include "Flappy_bird.h"
#include <atomic>
#include <thread>
#include <cinttypes>
#include <chrono>

namespace Render {
    class Display {
    private:
        static const unsigned int width = 1000;
        static const unsigned int height = 600;
        static const sf::String title;
        static const unsigned int frameRate = 60;

        static const int textGenerationX = 30;
        static const int textGenerationY = 115;

        static const int textFrameX = 30;
        static const int textFrameY = 150;

        static const int FlappyX = 250;
        static const int initFlappyY = 300;

        constexpr static const float flappyScale = 2.0;

        static const std::chrono::duration<int,std::milli> deltaTime;

        sf::RenderWindow window;
        sf::Text generationText;
        sf::Text frameText;
        sf::Sprite flappySprite;
        fb::Game gameInit;
    public:

        Display();

        sf::Image getImage()const;

        int renderEnv(const sf::Image &image, uint64_t generation, uint64_t frame);
    };

    void controllerLoop(std::atomic<bool>& exit, std::atomic<bool>& toggleDisplay,
                        std::atomic<bool>& doDisplay, const TPG::TPGVertex** bestRoot,
                        const Instructions::Set& set, Flappy_bird&,
                        const Learn::LearningParameters& params,
                        std::atomic<uint64_t>& generation);
    /**
	 * \brief Display the environment.
	 *
	 */


}
#endif //SFML_FLAPPY_BIRD_RENDER_H




