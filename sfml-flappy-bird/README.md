https://github.com/DenizBasgoren/sfml-flappy-bird
# sfml-flappy-bird
Flappy bird clone in C++ using the graphics library SFML

![screenshot](https://denizbasgoren.github.io/sfml-flappy-bird/screenshots/s1.png)

## How to install on Linux

First make sure that you have SFML 2+ installed. Then in the root folder of your git directory execute the following commands:

- `mkdir build`
- `cmake -S . -B build/`
- `cmake --build . --target <target>`
- `./<target>`

There is 2 possible targets :
- `flappy-bird_TPG` : Binary used to train a TPG to play flappy-bird.
- `flappy-bird_playable` : flappy is played by the user.


## How to install on Other Systems

First get rid of your Windows or whatever and obtain a modern GNU+Linux system image from [here](https://manjaro.org/). Then go to "How to install on Linux". If you insist to keep your non-Linux system, then watch the following tutorials:

- Windows: https://www.youtube.com/watch?v=GE-4hJDlmeA
- OSX: https://www.youtube.com/watch?v=mtEiyDbYMxQ

## Screenshots

![screenshot](https://denizbasgoren.github.io/sfml-flappy-bird/screenshots/s2.png)

![screenshot](https://denizbasgoren.github.io/sfml-flappy-bird/screenshots/s3.png)
