//
// Created by thomas on 19/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_RENDER_H
#define SFML_FLAPPY_BIRD_RENDER_H

#include "Flappy_bird.h"
#include <atomic>
#include <thread>
#include <cinttypes>

namespace Render {
    class Display {
    private:
        static const unsigned int width = 1000;
        static const unsigned int height = 600;
        static const sf::String title;
        static const unsigned int frameRate = 60;

        sf::RenderWindow window;
    public:

        sf::RenderWindow &getWindow() ;

    public:

        Display();

    };

    void controllerLoop(std::atomic<bool>& exit, std::atomic<bool>& toggleDisplay,
                        std::atomic<bool>& doDisplay, const TPG::TPGVertex** bestRoot,
                        const Instructions::Set& set, Flappy_bird&,
                        const Learn::LearningParameters& params,
                        std::atomic<uint64_t>& generation);

}
#endif //SFML_FLAPPY_BIRD_RENDER_H




