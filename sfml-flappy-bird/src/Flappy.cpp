//
// Created by thomas on 25/05/2021.
//

#include "Flappy.h"

fb::Flappy::Flappy(int x, int y) {
    v = 0;
    frame = 0;
    initX = x;
    initY = y;
    this->sprite.setPosition(x, y);
    this->sprite.setScale(flappyScale, flappyScale);
}

void fb::Flappy::reset() {
    v=0;
    frame = 0;

    this->sprite.setPosition(initX, initY);
}

fb::Flappy::Flappy(const Flappy &F):sprite(F.sprite) {
    this->initX = F.initX;
    this->initY = F.initY;

    this->v = F.v;
    this->frame = F.frame;

}
