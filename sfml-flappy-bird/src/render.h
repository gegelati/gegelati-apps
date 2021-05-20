//
// Created by thomas on 19/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_RENDER_H
#define SFML_FLAPPY_BIRD_RENDER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>

class Render  {
    // all sounds and their buffers will rest in this singleton struct

public:
    sf::RenderWindow window;
    struct Sounds {
        sf::SoundBuffer chingBuffer;
        sf::SoundBuffer hopBuffer;
        sf::SoundBuffer dishkBuffer;
        sf::Sound ching;
        sf::Sound hop;
        sf::Sound dishk;
    } sounds;

// all textures remain in here. Flappy has 3 textures, which will repeadetly draw, creating the illusion of flying.
    struct Textures {
        sf::Texture flappy[3];
        sf::Texture pipe;
        sf::Texture background;
        sf::Texture gameover;
    } textures;

    Render();

    Render(Render& ) = default;

    void initSound();

    void initTextures();
};



void loadSound();

#endif //SFML_FLAPPY_BIRD_RENDER_H
