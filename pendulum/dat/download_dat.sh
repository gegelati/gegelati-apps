#!/bin/bash

function log() {
	echo "$(date +%X) $1"
}

## dowload font
curl -o DejaVuSans.ttf https://preesm.github.io/assets/downloads/DejaVuSans.ttf

# download penduul
curl -o pendulum.png https://preesm.github.io/assets/downloads/pendulum.png

