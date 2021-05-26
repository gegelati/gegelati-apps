//
// Created by thomas on 25/05/2021.
//

#include "Texture.h"


fb::Texture::Texture(sf::String path) {
    this->background.loadFromFile(path+"/background.png");
    this->pipe.loadFromFile(path+"/pipe.png");
    this->gameover.loadFromFile(path+"/gameover.png");
    this->flappy[0].loadFromFile(path+"/flappy1.png");
    this->flappy[1].loadFromFile(path+"/flappy2.png");
    this->flappy[2].loadFromFile(path+"/flappy3.png");
}

const sf::Texture &fb::Texture::getFlappy(int i) const {
    return flappy[i];
}

const sf::Texture *fb::Texture::getFlappy() const {
    return flappy;
}

const sf::Texture &fb::Texture::getPipe() const {
    return pipe;
}

const sf::Texture &fb::Texture::getBackground() const {
    return background;
}

const sf::Texture &fb::Texture::getGameover() const {
    return gameover;
}

fb::Texture::Texture(const fb::Texture &t) {
    this->flappy[0] = t.flappy[0];
    this->flappy[1] = t.flappy[3];
    this->flappy[2] = t.flappy[2];

    this->gameover = t.gameover;
    this->background = t.background;
    this->pipe = t.pipe;
}

