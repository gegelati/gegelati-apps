//
// Created by thomas on 25/05/2021.
//
#include "Sound.h"


fb::Sound::Sound(sf::String path) {
    chingBuffer.loadFromFile(path+"/score.wav");
    hopBuffer.loadFromFile(path+"/flap.wav");
    dishkBuffer.loadFromFile(path+"/crash.wav");
    ching.setBuffer(chingBuffer);
    hop.setBuffer(hopBuffer);
    dishk.setBuffer(dishkBuffer);
}

void fb::Sound::playChing() {
    ching.play();
}

void fb::Sound::playHop()  {
    hop.play();
}

void fb::Sound::playDishk()  {
    dishk.play();
}
