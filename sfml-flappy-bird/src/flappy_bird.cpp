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
bool flappy_bird::collides(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    if (x1 + w1 >= x2 && x1 <= x2 + w2 && y1 + h1 >= y2 && y1 <= y2 + h2) {
        return true;
    }
    return false;
}

//void flappy_bird::initSound() {
//    // load sounds
//    sounds.chingBuffer.loadFromFile(ROOT_DIR "/dat/audio/score.wav");
//    sounds.hopBuffer.loadFromFile(ROOT_DIR "/dat/audio/flap.wav");
//    sounds.dishkBuffer.loadFromFile(ROOT_DIR "/dat/audio/crash.wav");
//    sounds.ching.setBuffer(sounds.chingBuffer);
//    sounds.hop.setBuffer(sounds.hopBuffer);
//    sounds.dishk.setBuffer(sounds.dishkBuffer);
//}

//void flappy_bird::initTextures() {
//    // load textures
//    textures.background.loadFromFile(ROOT_DIR "/dat/images/background.png");
//    textures.pipe.loadFromFile(ROOT_DIR "/dat/images/pipe.png");
//    textures.gameover.loadFromFile(ROOT_DIR "/dat/images/gameover.png");
//    textures.flappy[0].loadFromFile(ROOT_DIR "/dat/images/flappy1.png");
//    textures.flappy[1].loadFromFile(ROOT_DIR "/dat/images/flappy2.png");
//    textures.flappy[2].loadFromFile(ROOT_DIR "/dat/images/flappy3.png");
//}

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
    cout << endl << "--- Reset previous score : " << game.score << endl;

    this->nbActionsExecuted = 0;
    this->totalReward = 0.0;

    // update score
//    game.score = 0;
//    game.frames = 0;
//    game.scoreText.setString(to_string(game.score));
//    game.gameState = Game::GameState::started;
    game.reset();

//    flappy.sprite.setPosition(250, 300);
//    flappy.v = 0;
    flappy.reset();
    pipes.clear();

}

void flappy_bird::doAction(uint64_t actionID) {
    LearningEnvironment::doAction(actionID);
    nbActionsExecuted++;

    cout << actionID ;
    if(actionID == 1){
        if (game.gameState == Game::waiting) {
            game.gameState = Game::started;
        }

        if (game.gameState == Game::started) {
            flappy.v = -8;
//            game.getSound().playHop();
        }

    }
    
    // update score
    game.scoreText.setString(to_string(game.score));
    game.highscoreText.setString("HI " + to_string(game.highscore));

    // update flappy
    float fx = flappy.sprite.getPosition().x;
    float fy = flappy.sprite.getPosition().y;
    float fw = 34 * flappy.sprite.getScale().x;
    float fh = 24 * flappy.sprite.getScale().y;

    // flap the wings if playing
    if (game.gameState == Game::waiting || game.gameState == Game::started) {

        // change the texture once in 6 frames
        if (game.frames % 6 == 0) {
            flappy.frame += 1;
        }
        if (flappy.frame == 3) {
            flappy.frame = 0;
        }
    }

    flappy.sprite.setTexture(game.getTextures().getFlappy(flappy.frame));

    // move flappy
    if (game.gameState == Game::started) {
        flappy.sprite.move(0, flappy.v);
        flappy.v += 0.5;
    }
    // if hits ceiling, stop ascending
    // if out of screen, game over
    if (game.gameState == Game::started) {
        if (fy < 0) {
            flappy.sprite.setPosition(250, 0);
            flappy.v = 0;
        } else if (fy > 600) {
            flappy.v = 0;
            game.gameState = Game::gameover;
//				game.getSound().playDishk();
        }
    }

    // count the score
    for (vector<Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
        if (game.gameState == Game::started && (*itr).getPosition().x == 250) {
            game.score++;
//				game.getSound().playChing();

            if (game.score > game.highscore) {
                game.highscore = game.score;
            }

            break;
        }
    }

    // generate pipes
    if (game.gameState == Game::started && game.frames % 150 == 0) {
        int r = rng.getInt32(0,RAND_MAX) % 275 + 75;

        // lower pipe
        Sprite pipeL;
        pipeL.setTexture(game.getTextures().getPipe());
        pipeL.setPosition(1000, r + pipeGap);
        pipeL.setScale(pipeScaleX, pipeScaleY);

        // upper pipe
        Sprite pipeU;
        pipeU.setTexture(game.getTextures().getPipe());
        pipeU.setPosition(1000, r);
        pipeU.setScale(pipeScaleX, (-1*pipeScaleX));

        // push to the array
        pipes.push_back(pipeL);
        pipes.push_back(pipeU);
    }

    // move pipes
    if (game.gameState == Game::started) {
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

    // collision detection
    if (game.gameState == Game::started) {
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
                game.gameState = Game::gameover;
//                game.getSound().playDishk();
            }
        }
    }

    // clear, draw, display
    window.clear();
    window.draw(game.getBackground()[0]);
    window.draw(game.getBackground()[1]);
    window.draw(game.getBackground()[2]);
    window.draw(flappy.sprite);

    // draw pipes
    for (vector<Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
        window.draw(*itr);
    }

    // draw scores
    window.draw(game.scoreText);
    window.draw(game.highscoreText);

    // gameover. press c to continue
    if (game.gameState == Game::gameover) {
        window.draw(game.getGameoverSprite());

        if (game.frames % 60 < 30) {
            window.draw(game.getPressC());
        }
    }
    window.display();

    updateCurrentState();

    // dont forget to update total frames
    game.frames++;
}

bool flappy_bird::isCopyable() const {
    return true;
}

Learn::LearningEnvironment *flappy_bird::clone() const {
    return LearningEnvironment::clone();
}

bool flappy_bird::isTerminal() const {
    return (game.gameState == Game::GameState::gameover);
}

double flappy_bird::getScore() const {
    return game.frames;
}

flappy_bird::flappy_bird() :game(), Learn::LearningEnvironment(2), currentState(width*height*pixelLayer),
window(sf::VideoMode(width, height), "Floppy Bird"), rng(), flappy(initFlappyX, initFlappyY){
    window.setFramerateLimit(frameRate);
    window.setKeyRepeatEnabled(false);

//    initTextures();
//    initSound();

    // initial position, scale
//    flappy.sprite.setPosition(250, 300);
//    flappy.sprite.setScale(2, 2);



    // start score
//    game.scoreText.setString(to_string(game.frames));
//    game.highscoreText.setString("HI " + to_string(game.highscore));
    flappy.sprite.setTexture(game.getTextures().getFlappy(1));

    // clear, draw
    window.clear();
    window.draw(game.getBackground()[0]);
    window.draw(game.getBackground()[1]);
    window.draw(game.getBackground()[2]);
    window.draw(flappy.sprite);

    // draw scores
    window.draw(game.scoreText);
    window.draw(game.highscoreText);

    // display

    window.display();

    updateCurrentState();

}

void flappy_bird::updateCurrentState() {
    int luminance;
    sf::Image img = window.capture();

    const sf::Uint8* ptrImg = img.getPixelsPtr();
    for (unsigned int i = 0; i < (img.getSize().x * img.getSize().y * 4); i+=4) {
        /// Y = 0.299*R + 0.587*G + 0.114*B
        luminance = 0.299*(*ptrImg) + 0.587*(*(ptrImg+1)) + 0.114*(*(ptrImg+2));
        currentState.setDataAt(typeid(sf::Uint8), (int)(i/4), luminance);
        ptrImg+=4;
    }
}


