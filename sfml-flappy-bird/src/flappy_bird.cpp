//
// Created by thomas on 19/05/2021.
//

#include "flappy_bird.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "render.h"

using namespace sf;
using namespace std;

// rect rect collision detection helper function
bool collides(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    if (x1 + w1 >= x2 && x1 <= x2 + w2 && y1 + h1 >= y2 && y1 <= y2 + h2) {
        return true;
    }
    return false;
}

void flappy_bird::initSound() {
    // load sounds
    sounds.chingBuffer.loadFromFile(ROOT_DIR "/dat/audio/score.wav");
    sounds.hopBuffer.loadFromFile(ROOT_DIR "/dat/audio/flap.wav");
    sounds.dishkBuffer.loadFromFile(ROOT_DIR "/dat/audio/crash.wav");
    sounds.ching.setBuffer(sounds.chingBuffer);
    sounds.hop.setBuffer(sounds.hopBuffer);
    sounds.dishk.setBuffer(sounds.dishkBuffer);
}

void flappy_bird::initTextures() {
    // load textures
    textures.background.loadFromFile(ROOT_DIR "/dat/images/background.png");
    textures.pipe.loadFromFile(ROOT_DIR "/dat/images/pipe.png");
    textures.gameover.loadFromFile(ROOT_DIR "/dat/images/gameover.png");
    textures.flappy[0].loadFromFile(ROOT_DIR "/dat/images/flappy1.png");
    textures.flappy[1].loadFromFile(ROOT_DIR "/dat/images/flappy2.png");
    textures.flappy[2].loadFromFile(ROOT_DIR "/dat/images/flappy3.png");
}

std::vector<std::reference_wrapper<const Data::DataHandler>> flappy_bird::getDataSources() {
    auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
    result.push_back(currentState);
    return result;
}

void flappy_bird::reset(size_t seed, Learn::LearningMode mode) {
    // Create seed from seed and mode
    size_t hash_seed = Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);

    // Reset the RNG
    rng.setSeed(hash_seed);

    // initial position, scale
    flappy.sprite.setPosition(250, 300);
    flappy.sprite.setScale(2, 2);

    this->nbActionsExecuted = 0;
    this->totalReward = 0.0;
    game.gameState = waiting;

    // update score
    flappy.sprite.setTexture(textures.flappy[1]);
    game.scoreText.setString(to_string(game.score));
    game.highscoreText.setString("HI " + to_string(game.highscore));

    // update flappy
    float fx = flappy.sprite.getPosition().x;
    float fy = flappy.sprite.getPosition().y;
    float fw = 34 * flappy.sprite.getScale().x;
    float fh = 24 * flappy.sprite.getScale().y;
}

void flappy_bird::doAction(uint64_t actionID) {
    LearningEnvironment::doAction(actionID);

    if(actionID == 1){
        if (game.gameState == waiting) {
            game.gameState = started;
        }

        if (game.gameState == started) {
            flappy.v = -8;
//            sounds.hop.play();
        }

    }
    
    // update score
    flappy.sprite.setTexture(textures.flappy[1]);
    game.scoreText.setString(to_string(game.score));
    game.highscoreText.setString("HI " + to_string(game.highscore));

    // update flappy
    float fx = flappy.sprite.getPosition().x;
    float fy = flappy.sprite.getPosition().y;
    float fw = 34 * flappy.sprite.getScale().x;
    float fh = 24 * flappy.sprite.getScale().y;

    // flap the wings if playing
    if (game.gameState == waiting || game.gameState == started) {

        // change the texture once in 6 frames
        if (game.frames % 6 == 0) {
            flappy.frame += 1;
        }
        if (flappy.frame == 3) {
            flappy.frame = 0;
        }
    }

    flappy.sprite.setTexture(textures.flappy[flappy.frame]);

    // move flappy
    if (game.gameState == started) {
        flappy.sprite.move(0, flappy.v);
        flappy.v += 0.5;
    }
    // if hits ceiling, stop ascending
    // if out of screen, game over
    if (game.gameState == started) {
        if (fy < 0) {
            flappy.sprite.setPosition(250, 0);
            flappy.v = 0;
        } else if (fy > 600) {
            flappy.v = 0;
            game.gameState = gameover;
//				sounds.dishk.play();
        }
    }

    // count the score
    for (vector<Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
        if (game.gameState == started && (*itr).getPosition().x == 250) {
            game.score++;
//				sounds.ching.play();

            if (game.score > game.highscore) {
                game.highscore = game.score;
            }

            break;
        }
    }

    // generate pipes
    if (game.gameState == started && game.frames % 150 == 0) {
        int r = rng.getInt32(0,RAND_MAX) % 275 + 75;
        int gap = 150;

        // lower pipe
        Sprite pipeL;
        pipeL.setTexture(textures.pipe);
        pipeL.setPosition(1000, r + gap);
        pipeL.setScale(2, 2);

        // upper pipe
        Sprite pipeU;
        pipeU.setTexture(textures.pipe);
        pipeU.setPosition(1000, r);
        pipeU.setScale(2, -2);

        // push to the array
        pipes.push_back(pipeL);
        pipes.push_back(pipeU);
    }

    // move pipes
    if (game.gameState == started) {
        for (vector<Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
            (*itr).move(-3, 0);
        }
    }

    // remove pipes if offscreen
    if (game.frames % 100 == 0) {
        vector<Sprite>::iterator startitr = pipes.begin();
        vector<Sprite>::iterator enditr = pipes.begin();

        for (; enditr != pipes.end(); enditr++) {
            if ((*enditr).getPosition().x > -104) {
                break;
            }
        }

        pipes.erase(startitr, enditr);
    }

    ///currentState = window.capture();
    // collision detection
    if (game.gameState == started) {
        for (vector<Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {

            float px, py, pw, ph;
// l'origine est en haut à gauche avec x abscisse et y l'ordonée
            if ((*itr).getScale().y > 0) {
                px = (*itr).getPosition().x;
                py = (*itr).getPosition().y;
                pw = 52 * (*itr).getScale().x;
                ph = 320 * (*itr).getScale().y;
            } else {
                pw = 52 * (*itr).getScale().x;
                ph = -320 * (*itr).getScale().y;
                px = (*itr).getPosition().x;
                py = (*itr).getPosition().y - ph;
            }

            if (collides(fx, fy, fw, fh, px, py, pw, ph)) {
                game.gameState = gameover;
//                sounds.dishk.play();
            }
        }
    }

}

bool flappy_bird::isCopyable() const {
    return true;
}

Learn::LearningEnvironment *flappy_bird::clone() const {
    return LearningEnvironment::clone();
}

bool flappy_bird::isTerminal() const {
    return (game.gameState == gameover);
}

double flappy_bird::getScore() const {
    return game.score;
}

flappy_bird::flappy_bird() : Learn::LearningEnvironment(2), currentState(width*height*pixelLayer){
    sf::RenderWindow window(sf::VideoMode(width, height), "Floppy Bird");
    window.setFramerateLimit(frameRate);
    window.setKeyRepeatEnabled(false);

    initTextures();
    initSound();

    game.font.loadFromFile(ROOT_DIR "/dat/fonts/flappy.ttf");
    game.background[0].setTexture(textures.background);
    game.background[1].setTexture(textures.background);
    game.background[2].setTexture(textures.background);
    game.background[0].setScale(1.15625, 1.171875);
    game.background[1].setScale(1.15625, 1.171875);
    game.background[2].setScale(1.15625, 1.171875);
    game.background[1].setPosition(333, 0);
    game.background[2].setPosition(666, 0);
    game.gameover.setTexture(textures.gameover);
    game.gameover.setOrigin(192 / 2, 42 / 2);
    game.gameover.setPosition(500, 125);
    game.gameover.setScale(2, 2);
    game.pressC.setString("Press C to continue");
    game.pressC.setFont(game.font);
    game.pressC.setFillColor(sf::Color::White);
    game.pressC.setCharacterSize(50);
    game.pressC.setOrigin(game.pressC.getLocalBounds().width / 2, 0);
    game.pressC.setPosition(500, 250);
    game.scoreText.setFont(game.font);
    game.scoreText.setFillColor(sf::Color::White);
    game.scoreText.setCharacterSize(75);
    game.scoreText.move(30, 0);
    game.highscoreText.setFont(game.font);
    game.highscoreText.setFillColor(sf::Color::White);
    game.highscoreText.move(30, 80);

    // start score
    flappy.sprite.setTexture(textures.flappy[1]);
    game.scoreText.setString(to_string(game.score));
    game.highscoreText.setString("HI " + to_string(game.highscore));

    window.display();


    sf::Image img = window.capture();
    ptrVectImg = new std::vector<sf::Uint8>;

    const sf::Uint8* ptrImg = img.getPixelsPtr();
    for (unsigned int i = 0; i < (img.getSize().x * img.getSize().y * 4); ++i) {
        currentState.setDataAt(typeid(sf::Uint8), i, *ptrImg);
        ptrImg++;
    }

}


