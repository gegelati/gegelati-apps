//
// Created by thomas on 25/05/2021.
//
#include "Sound.h"

Sound::Sound(sf::String path) {
    chingBuffer.loadFromFile(path+"/score.wav");
    hopBuffer.loadFromFile(path+"/flap.wav");
    dishkBuffer.loadFromFile(path+"/crash.wav");
    ching.setBuffer(chingBuffer);
    hop.setBuffer(hopBuffer);
    dishk.setBuffer(dishkBuffer);
}

void Sound::playChing() {
    ching.play();
}

void Sound::playHop()  {
    hop.play();
}

void Sound::playDishk()  {
    dishk.play();
}
