//
// Created by thomas on 19/05/2021.
//

#include "Flappy_bird.h"
#include <iostream>


// rect rect collision detection helper function
bool Flappy_bird::collides(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    if (x1 + w1 >= x2 && x1 <= x2 + w2 && y1 + h1 >= y2 && y1 <= y2 + h2) {
        return true;
    }
    return false;
}

std::vector<std::reference_wrapper<const Data::DataHandler>> Flappy_bird::getDataSources() {
    auto result = std::vector<std::reference_wrapper<const Data::DataHandler>>();
    result.push_back(currentState);
    return result;
}

void Flappy_bird::reset(size_t seed, Learn::LearningMode mode) {
    // Create seed from seed and mode
    size_t hash_seed = Data::Hash<size_t>()(seed) ^ Data::Hash<Learn::LearningMode>()(mode);

    // Reset the RNG
    rng.setSeed(hash_seed);
    std::cout << std::endl << "--- Reset previous score : " << totalReward << std::endl;

    this->totalReward = 0;

    game.reset();
    flappy.reset();
    pipes.clear();
    game.scoreText.setString(std::to_string(totalReward));
    img.display();
    updateCurrentState();
}

void Flappy_bird::doAction(uint64_t actionID) {
    LearningEnvironment::doAction(actionID);

    std::cout << actionID ;
    if(actionID == 1){
        if (game.gameState == fb::Game::started) {
            flappy.v = -8;
//            game.getSound().playHop();
        }

    }
    
    // update score
    game.scoreText.setString(std::to_string(totalReward));
    game.highscoreText.setString("HI " + std::to_string(game.highscore));

    // update flappy
    float fx = flappy.sprite.getPosition().x;
    float fy = flappy.sprite.getPosition().y;
    float fw = 34 * flappy.sprite.getScale().x;
    float fh = 24 * flappy.sprite.getScale().y;

    // flap the wings if playing
    if (game.gameState == fb::Game::started) {

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
    if (game.gameState == fb::Game::started) {
        flappy.sprite.move(0, flappy.v);
        flappy.v += 0.5;
    }
    // if hits ceiling, stop ascending
    // if out of screen, game over
    if (game.gameState == fb::Game::started) {
        if (fy < 0) {
            flappy.sprite.setPosition(FlappyX, 0);
            flappy.v = 0;
        } else if (fy > 600) {
            flappy.v = 0;
            game.gameState = fb::Game::gameover;
//				game.getSound().playDishk();
        }
    }

    // count the score
/// total_reward = distance + coeff * pipes = game.score+ multiplierRewardPipe * game.pipe
    if (game.gameState == fb::Game::started) {
        totalReward++;
        for (std::vector<sf::Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
            if ((*itr).getPosition().x == 250) {
                totalReward += multiplierRewardPipe;
//				game.getSound().playChing();

                if (totalReward > game.highscore) {
                    game.highscore = totalReward;
                }

                break;
            }
        }
    }

    // generate pipes
    if (game.gameState == fb::Game::started && game.frames % 150 == 0) {
        int r = rng.getInt32(0,RAND_MAX) % 275 + 75;

        // lower pipe
        sf::Sprite pipeL;
        pipeL.setTexture(game.getTextures().getPipe());
        pipeL.setPosition(1000, r + pipeGap);
        pipeL.setScale(pipeScaleX, pipeScaleY);

        // upper pipe
        sf::Sprite pipeU;
        pipeU.setTexture(game.getTextures().getPipe());
        pipeU.setPosition(1000, r);
        pipeU.setScale(pipeScaleX, (-1*pipeScaleX));

        // push to the array
        pipes.push_back(pipeL);
        pipes.push_back(pipeU);
    }

    // move pipes
    if (game.gameState == fb::Game::started) {
        for (std::vector<sf::Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
            (*itr).move(-3, 0);
        }
    }

    // remove pipes if offscreen
    if (game.frames % 100 == 0) {
        std::vector<sf::Sprite>::iterator startitr = pipes.begin();
        std::vector<sf::Sprite>::iterator enditr = pipes.begin();

        for (; enditr != pipes.end(); enditr++) {
            if ((*enditr).getPosition().x > -104) {
                break;
            }
        }

        pipes.erase(startitr, enditr);
    }

    // collision detection
    if (game.gameState == fb::Game::started) {
        for (std::vector<sf::Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {

            float px, py, pw, ph;
// The origin is on the top left corner with x horizontal et y vertical
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
                game.gameState = fb::Game::gameover;
//                game.getSound().playDishk();
            }
        }
    }

    // clear, draw, display
    img.clear();
    img.draw(game.getBackground()[0]);
    img.draw(game.getBackground()[1]);
    img.draw(game.getBackground()[2]);
    img.draw(flappy.sprite);

    // draw pipes
    for (std::vector<sf::Sprite>::iterator itr = pipes.begin(); itr != pipes.end(); itr++) {
        img.draw(*itr);
    }

    // draw scores
    img.draw(game.scoreText);
    img.draw(game.highscoreText);

    // gameover. press c to continue
    if (game.gameState == fb::Game::gameover) {
        img.draw(game.getGameoverSprite());

        if (game.frames % 60 < 30) {
            img.draw(game.getPressC());
        }
    }
    img.display();

    updateCurrentState();

    // dont forget to update total frames
    game.frames++;
}

bool Flappy_bird::isCopyable() const {
    return true;
}

Flappy_bird::Flappy_bird(const Flappy_bird &f): Learn::LearningEnvironment(f.nbActions), img(),
game(f.game), currentState(f.currentState), picture(f.picture), rng(f.rng), flappy(FlappyX, initFlappyY),
pipes(f.pipes){
    this->img.draw(sf::Sprite(f.img.getTexture()));
    this->img.display();
}

Learn::LearningEnvironment *Flappy_bird::clone() const {
    Learn::LearningEnvironment* ptr = new Flappy_bird(*this);
    return ptr;
}

bool Flappy_bird::isTerminal() const {
    return (game.gameState == fb::Game::GameState::gameover);
}

double Flappy_bird::getScore() const {
    return totalReward;
}

Flappy_bird::Flappy_bird() :game(), Learn::LearningEnvironment(2), currentState(width*height*pixelLayer),
img(), rng(), flappy(FlappyX, initFlappyY), pipes(){
    flappy.sprite.setTexture(game.getTextures().getFlappy(1));
    img.create(width, height);
//    window.setFramerateLimit(frameRate);
//    window.setKeyRepeatEnabled(false);


    // clear, draw
    img.clear();
    img.draw(game.getBackground()[0]);
    img.draw(game.getBackground()[1]);
    img.draw(game.getBackground()[2]);
    img.draw(flappy.sprite);

    // draw scores
    img.draw(game.scoreText);
    img.draw(game.highscoreText);

    // display
    img.display();

    updateCurrentState();

}

void Flappy_bird::updateCurrentState() {
    int luminance;
    picture =  img.getTexture().copyToImage();

    const sf::Uint8* ptrImg = picture.getPixelsPtr();
    for (unsigned int i = 0; i < (picture.getSize().x * picture.getSize().y * 4); i+=4) {
        /// Y = 0.299*R + 0.587*G + 0.114*B
        luminance = (int)(0.299*(*ptrImg) + 0.587*(*(ptrImg+1)) + 0.114*(*(ptrImg+2)));
        currentState.setDataAt(typeid(sf::Uint8), (int)(i/4), luminance);
        ptrImg+=4;
    }
    //TODO zqd
}


