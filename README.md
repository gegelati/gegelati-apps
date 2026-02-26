# GEGELATI Applications

The purpose of this repository is to gather several applications built with the [GEGELATI library](https://github.com/gegelati/gegelati),and manage their CI.

## Apps overview

| App name  | Description                           | Lib. version |
| --------- | ------------------------------------- | -------------|
| MNIST     | Classification of handwritten digits. | 2.0.0        |
| Pendulum  | Control of an inverted pendulum.      | latest       |
| Stickgame | Nim game with 21 sticks.              | 2.0.0        |
| TicTacToe | Tic Tac Toe game.                     | 2.0.0        |

When a lib version is specified, the app won't be updated to the latest version of the library, and will be tested with the specified version. When "latest" is specified, the app will be updated to the latest version of the library, and will be tested with it.