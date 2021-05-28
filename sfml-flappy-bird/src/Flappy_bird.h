//
// Created by thomas on 19/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_FLAPPY_BIRD_H
#define SFML_FLAPPY_BIRD_FLAPPY_BIRD_H

#include <gegelati.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "data/primitiveTypeArray.h"
#include "mutator/rng.h"
#include "learn/adversarialLearningEnvironment.h"
#include "Flappy.h"
#include "Game.h"
#include <iostream>
#include <search.h>


class Flappy_bird : public Learn::LearningEnvironment{
private:
    static const unsigned int multiplierRewardPipe = 50;

    static const unsigned int width = 1000;
    static const unsigned int height = 600;
    static const unsigned int pixelLayer = 1;

    static const unsigned int pipeGap = 150;
    constexpr static const float pipeScaleX = 2.0;
    constexpr static const float pipeScaleY = 2.0;

    static const int FlappyX = 250;
    static const int initFlappyY = 300;

    /// Total reward accumulated since the last reset
    int totalReward = 0;

    /// Image
    Data::PrimitiveTypeArray<sf::Uint8> currentState;
    sf::Image picture;
    sf::Sprite spr;

    /// Randomness control
    Mutator::RNG rng;

    fb::Flappy flappy;

    fb::Game game;

    sf::RenderTexture img;

    std::vector<sf::Sprite> pipes;

    std::vector<int> vectActions;

    bool collides(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

    void updateCurrentState();
public:

    sf::Sprite getSprite() const;

    sf::Image getPicture() const;

    Flappy_bird();

    Flappy_bird(const Flappy_bird&);

    /// Inherited via LearningEnvironment
    virtual std::vector<std::reference_wrapper<const Data::DataHandler>> getDataSources() override;

    /// Inherited via LearningEnvironment
    void reset(size_t seed = 0, Learn::LearningMode mode = Learn::LearningMode::TRAINING) override;

    /// Inherited via LearningEnvironment
    void doAction(uint64_t actionID) override;

    /// Inherited via LearningEnvironment
    virtual bool isCopyable() const override;

    /// Inherited via LearningEnvironment
    virtual LearningEnvironment* clone() const;

    virtual double getScore() const override;

    /// Inherited via LearningEnvironment
    virtual bool isTerminal() const override;

};

void print(sf::Image pic);
void print(sf::Sprite spr);
void print(sf::Texture tex);

#endif //SFML_FLAPPY_BIRD_FLAPPY_BIRD_H
