//
// Created by thomas on 25/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_FLAPPY_H
#define SFML_FLAPPY_BIRD_FLAPPY_H

#include <SFML/Graphics.hpp>

namespace fb {
    class Flappy {
    private:
        constexpr static const float flappyScale = 2.0;
        int initX, initY;

    public:
        double v;
        int frame;
        sf::Sprite sprite;

        Flappy(int x, int y);

        Flappy(const Flappy& );

        void reset();
    };
}


#endif //SFML_FLAPPY_BIRD_FLAPPY_H
