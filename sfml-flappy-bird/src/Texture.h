//
// Created by thomas on 25/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_TEXTURE_H
#define SFML_FLAPPY_BIRD_TEXTURE_H

#include <SFML/Graphics.hpp>

class Texture {
private:
    sf::Texture flappy[3];
    sf::Texture pipe;
    sf::Texture background;
    sf::Texture gameover;

public:
    const sf::Texture &getFlappy(int i) const;

    const sf::Texture* getFlappy() const;

    const sf::Texture &getPipe() const;

    const sf::Texture &getBackground() const;

    const sf::Texture &getGameover() const;

    Texture(sf::String path);
};


#endif //SFML_FLAPPY_BIRD_TEXTURE_H
