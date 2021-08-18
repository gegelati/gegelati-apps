# Pendulum

This application teaches a learning agent built with the [GEGELATI library](https://github.com/gegelati/gegelati) how to keep a pendulum at an equilibrium position.

## How to Build?
The build process of applications relies on [cmake](https://cmake.org) to configure a project for a wide variety of development environments and operating systems. Install [cmake](https://cmake.org/download/) on your system before building the application.

### Under windows
1. Copy the `gegelatilib-<version>` folder containing the binaries of the [GEGELATI library](https://github.com/gegelati/gegelati) into the `lib` folder.
2. Open a command line interface in the `bin` folder.
3. Enter the following command to create the project for your favorite IDE `cmake ..`.
4. Open the project created in the `bin` folder, or launch the build with the following command: `cmake --build .`.

## CodeGen example

The folder src/CodeGen contains an example of use case for the code gen. There are 3 targets for this example, you can directly run the last one :
- PendulumCodeGen that import the TPG_graph.dot and launch the code gen to generate the sources files. If you want to run this target you need to set your working directory as the current build directory of your build system. You can use the following variable $CMakeCurrentBuildDir$.
- PendulumExecCodeGen that is a custom command to execute the previous target 
- PendulumInferenceCodeGen that use the generated file and link them with the learning environment of the directory src/Learn. 