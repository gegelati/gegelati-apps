//
// Created by thomas on 19/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_FLAPPY_BIRD_H
#define SFML_FLAPPY_BIRD_FLAPPY_BIRD_H

#include <gegelati.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include "data/primitiveTypeArray.h"
#include "mutator/rng.h"
#include "learn/adversarialLearningEnvironment.h"
#include "render.h"

class flappy_bird : public Learn::LearningEnvironment{
private:
    static const unsigned int width = 1000;
    static const unsigned int height = 600;
    static const unsigned int pixelLayer = 1;
    static const unsigned int frameRate = 60;

    static const size_t REWARD_HISTORY_SIZE = 300;

    /// Reward history for score computation
    double rewardHistory[REWARD_HISTORY_SIZE];

    /// Total reward accumulated since the last reset
    double totalReward = 0.0;

    /// Number of actions since the last reset
    uint64_t nbActionsExecuted = 0;

    /// Image
    Data::PrimitiveTypeArray<sf::Uint8> currentState;

    /// Randomness control
    Mutator::RNG rng;

    bool collides(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);

    void updateCurrentState();
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

    struct Flappy {
        double v = 0;
        int frame = 0;
        sf::Sprite sprite;
    } flappy;

    enum GameState { waiting, started, gameover };

    struct Game {
        int score = 0;
        int highscore = 0;
        int frames = 0;
        GameState gameState = waiting;
        sf::Sprite background[3];
        sf::Sprite gameover;
        sf::Text pressC;
        sf::Text scoreText;
        sf::Text highscoreText;
        sf::Font font;
    } game;

    std::vector<sf::Sprite> pipes;

    void initSound();

    void initTextures();

    flappy_bird();

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


#endif //SFML_FLAPPY_BIRD_FLAPPY_BIRD_H
