//
// Created by thomas on 25/05/2021.
//

#include "Game.h"

using namespace std;

const sf::String Game::pathImage = ROOT_DIR "/dat/images";
const sf::String Game::pathSound = ROOT_DIR "/dat/audio";
const sf::String Game::FontFile = ROOT_DIR "/dat/fonts";

Game::Game(): textures(pathImage), sound(pathSound){

    highscore = 0;
    frames = 0;
    score = 0;
    gameState = GameState::waiting;


    scoreText.setString(to_string(frames));
    highscoreText.setString("HI " + to_string(highscore));

    font.loadFromFile(FontFile+"/flappy.ttf");
    background[0].setTexture(textures.getBackground());
    background[1].setTexture(textures.getBackground());
    background[2].setTexture(textures.getBackground());
    background[0].setScale(backgroundScaleX, backgroundScaleY);
    background[1].setScale(backgroundScaleX, backgroundScaleY);
    background[2].setScale(backgroundScaleX, backgroundScaleY);
    background[1].setPosition(backgroundX[1], backgroundY[1]);
    background[2].setPosition(backgroundX[2], backgroundY[2]);

    gameoverSprite.setTexture(textures.getGameover());
    gameoverSprite.setOrigin(192 / 2, 42 / 2);
    gameoverSprite.setPosition(gameoverSpriteX, gameoverSpriteY);
    gameoverSprite.setScale(gameoverSpriteScale, gameoverSpriteScale);

    pressC.setString("Press C to continue");
    pressC.setFont(font);
    pressC.setFillColor(sf::Color::White);
    pressC.setCharacterSize(CharacterSizePressC);
    pressC.setOrigin(pressC.getLocalBounds().width / 2, 0);
    pressC.setPosition(pressCSpriteX, pressCSpriteY);

    scoreText.setFont(font);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setCharacterSize(CharacterSizeScores);
    scoreText.move(textScoreX, textScoreY);

    highscoreText.setFont(font);
    highscoreText.setFillColor(sf::Color::White);
    highscoreText.move(textHighScoreX, textHighScoreY);

}

const Texture &Game::getTextures() const {
    return textures;
}

Sound Game::getSound() const{
    return sound;
}

const sf::Sprite *Game::getBackground() const {
    return background;
}

const sf::Sprite &Game::getGameoverSprite() const {
    return gameoverSprite;
}

const sf::Text &Game::getPressC() const {
    return pressC;
}

void Game::reset() {
    frames = 0;
    score = 0;
    scoreText.setString(to_string(score));
    gameState = Game::GameState::started;

}
