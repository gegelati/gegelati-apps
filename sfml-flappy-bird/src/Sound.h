//
// Created by thomas on 25/05/2021.
//

#ifndef SFML_FLAPPY_BIRD_SOUND_H
#define SFML_FLAPPY_BIRD_SOUND_H

#include <SFML/Audio.hpp>

namespace fb {
    class Sound {
    private:
        sf::SoundBuffer chingBuffer;
        sf::SoundBuffer hopBuffer;
        sf::SoundBuffer dishkBuffer;

        sf::Sound ching;
        sf::Sound hop;
        sf::Sound dishk;

    public:
        void playChing();

        void playHop();

        void playDishk();

        Sound(sf::String path);

        Sound(const Sound &) = default;
    };
}


#endif //SFML_FLAPPY_BIRD_SOUND_H
