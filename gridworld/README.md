# GridWorld

This application teaches a learning agent built with the [GEGELATI library](https://github.com/gegelati/gegelati) how to get out of a gridWorld

The gridWorld is a grid composed of 0, 1, 2, and 3. 
 * 0 is an available tile
 * 1 is a good output tile
 * 2 is a bad output tile
 * 3 is unavailable tile

The agent start at coordonate (0, 0). It can go left, right, up and down.

It get a reward of -1 if it reach a tile with value 0. 
If it reach a tile a value 1 or 2, it terminate the environnement and the agent get a reward of respectively 100 or -100.

## How to Build?
The build process of applications relies on [cmake](https://cmake.org) to configure a project for a wide variety of development environments and operating systems. Install [cmake](https://cmake.org/download/) on your system before building the application.

### Under windows
1. Copy the `gegelatilib-<version>` folder containing the binaries of the [GEGELATI library](https://github.com/gegelati/gegelati) into the `lib` folder.
2. Open a command line interface in the `bin` folder.
3. Enter the following command to create the project for your favorite IDE `cmake ..`.
4. Open the project created in the `bin` folder, or launch the build with the following command: `cmake --build .`.
