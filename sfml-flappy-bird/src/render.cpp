//
// Created by thomas on 19/05/2021.
//

#include "render.h"

void loadSound() {

}

void Render::initSound() {
    // load sounds
    sounds.chingBuffer.loadFromFile("../../dat/audio/score.wav");
    sounds.hopBuffer.loadFromFile("../../dat/audio/flap.wav");
    sounds.dishkBuffer.loadFromFile("../../dat/audio/crash.wav");
    sounds.ching.setBuffer(sounds.chingBuffer);
    sounds.hop.setBuffer(sounds.hopBuffer);
    sounds.dishk.setBuffer(sounds.dishkBuffer);
}

void Render::initTextures() {
    // load textures
    textures.background.loadFromFile("../../dat/images/background.png");
    textures.pipe.loadFromFile("../../dat/images/pipe.png");
    textures.gameover.loadFromFile("../../dat/images/gameover.png");
    textures.flappy[0].loadFromFile("../../dat/images/flappy1.png");
    textures.flappy[1].loadFromFile("../../dat/images/flappy2.png");
    textures.flappy[2].loadFromFile("../../dat/images/flappy3.png");
}

Render::Render() {
    // create the window and set general settings.
    sf::RenderWindow window(sf::VideoMode(1000, 600), "Floppy Bird");
	window.setFramerateLimit(90);
	window.setKeyRepeatEnabled(false);

    initTextures();
    initSound();

}


