//
// Created by thomas on 25/05/2021.
//

#include "Game.h"

const sf::String fb::Game::pathImage = ROOT_DIR "/dat/images";
const sf::String fb::Game::pathSound = ROOT_DIR "/dat/audio";
const sf::String fb::Game::FontFile = ROOT_DIR "/dat/fonts";

fb::Game::Game(): textures(pathImage)/*, sound(pathSound)*/{

    highscore = 0;
    frames = 0;
    gameState = GameState::started;


    scoreText.setString(std::to_string(frames));
    highscoreText.setString("HI " + std::to_string(highscore));

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

const fb::Texture &fb::Game::getTextures() const {
    return textures;
}

//fb::Sound fb::Game::getSound() const{
//    return sound;
//}

const sf::Sprite *fb::Game::getBackground() const {
    return background;
}

const sf::Sprite &fb::Game::getGameoverSprite() const {
    return gameoverSprite;
}

const sf::Text &fb::Game::getPressC() const {
    return pressC;
}

void fb::Game::reset() {
    frames = 0;
    gameState = Game::GameState::started;

}

fb::Game::Game(const Game &g):gameState(g.gameState), scoreText(g.scoreText), highscoreText(g.highscoreText),
frames(g.frames), highscore(g.highscore), textures(g.textures)/*, sound(g.sound)*/, gameoverSprite(g.gameoverSprite),
pressC(g.pressC), font(g.font) {
    this->background[0] = g.background[0];
    this->background[1] = g.background[1];
    this->background[2] = g.background[2];
}

const sf::Font &fb::Game::getFont() const {
    return font;
}
