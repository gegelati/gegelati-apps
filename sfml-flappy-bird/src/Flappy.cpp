//
// Created by thomas on 25/05/2021.
//

#include "Flappy.h"

Flappy::Flappy(int x, int y) {
    v = 0;
    frame = 0;
    initX = x;
    initY = y;
    this->sprite.setPosition(x, y);
    this->sprite.setScale(flappyScale, flappyScale);
}

void Flappy::reset() {
    v=0;
    frame = 0;

    this->sprite.setPosition(initX, initY);
}