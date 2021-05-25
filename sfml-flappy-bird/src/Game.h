//
// Created by thomas on 25/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_GAME_H
#define SFML_FLAPPY_BIRD_GAME_H

#include <SFML/Graphics.hpp>
#include "Texture.h"
#include "Sound.h"

class Game {
private:
    constexpr static const float backgroundScaleX = 1.15625;
    constexpr static const float backgroundScaleY = 1.171875;
    constexpr const static int backgroundX[] = {0, 333, 666};
    constexpr const static int backgroundY[] = {0, 0, 0};

    constexpr static const float gameoverSpriteScale = 2.0;
    static const int gameoverSpriteX = 500;
    static const int gameoverSpriteY = 125;

    static const int pressCSpriteX = 500;
    static const int pressCSpriteY = 250;
    static const unsigned int CharacterSizePressC = 50;
    static const unsigned int CharacterSizeScores = 75;

    static const int textScoreX = 30;
    static const int textScoreY = 0;

    static const int textHighScoreX = 30;
    static const int textHighScoreY = 80;

public:
    enum GameState { waiting, started, gameover };
    GameState gameState;
    sf::Text scoreText;
    sf::Text highscoreText;
    int highscore;
    int frames;
    int score;

private:
    static const sf::String pathImage;
    static const sf::String pathSound;
    static const sf::String FontFile;

    Texture textures;
    Sound sound;

    sf::Sprite background[3];
    sf::Sprite gameoverSprite;
    sf::Text pressC;
    sf::Font font;

public:

    void reset();

    Game();

    const sf::Sprite* getBackground() const;

    const sf::Sprite &getGameoverSprite() const;

    const Texture &getTextures() const;

    Sound getSound() const;

    const sf::Text &getPressC() const;
};


#endif //SFML_FLAPPY_BIRD_GAME_H
