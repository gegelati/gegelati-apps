# Tic tac toe game

This application teaches a learning agent built with the [GEGELATI library](https://github.com/gegelati/gegelati) how to play (and win) to the Tic-Tac-Toe game (https://en.wikipedia.org/wiki/Tic-Tac-Toe).

## How to Build?
The build process of applications relies on [cmake](https://cmake.org) to configure a project for a wide variety of development environments and operating systems. Install [cmake](https://cmake.org/download/) on your system before building the application.

### Under windows
1. Copy the `gegelatilib-<version>` folder containing the binaries of the [GEGELATI library](https://github.com/gegelati/gegelati) into the `lib` folder.
2. Open a command line interface in the `bin` folder.
3. Enter the following command to create the project for your favorite IDE `cmake ..`.
4. Open the project created in the `bin` folder, or launch the build with the following command: `cmake --build .`.

