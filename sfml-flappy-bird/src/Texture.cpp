//
// Created by thomas on 25/05/2021.
//

#include "Texture.h"

Texture::Texture(sf::String path) {
    this->background.loadFromFile(path+"/background.png");
    this->pipe.loadFromFile(path+"/pipe.png");
    this->gameover.loadFromFile(path+"/gameover.png");
    this->flappy[0].loadFromFile(path+"/flappy1.png");
    this->flappy[1].loadFromFile(path+"/flappy2.png");
    this->flappy[2].loadFromFile(path+"/flappy3.png");
}

const sf::Texture &Texture::getFlappy(int i) const {
    return flappy[i];
}

const sf::Texture *Texture::getFlappy() const {
    return flappy;
}

const sf::Texture &Texture::getPipe() const {
    return pipe;
}

const sf::Texture &Texture::getBackground() const {
    return background;
}

const sf::Texture &Texture::getGameover() const {
    return gameover;
}

