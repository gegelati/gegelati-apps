#!/bin/bash

# Update and install wget and unzip if necessary
sudo apt update
sudo apt install -y wget unzip

# Create the directory for the project
LIB_DIR=$(pwd)

# MuJoCo version
MUJOCO_VERSION=210

# Download MuJoCo
echo "Downloading MuJoCo..."
MUJOCO_URL="https://mujoco.org/download/mujoco${MUJOCO_VERSION}-linux-x86_64.tar.gz"
wget -O mujoco${MUJOCO_VERSION}-linux-x86_64.tar.gz "$MUJOCO_URL"

# Check if the download was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to download MuJoCo."
    exit 1
fi

# Extract the downloaded file
echo "Extracting MuJoCo..."
tar -xzf mujoco${MUJOCO_VERSION}-linux-x86_64.tar.gz

# Check if the extraction was successful
if [ $? -ne 0 ]; then
    echo "Error: Failed to extract MuJoCo."
    rm mujoco${MUJOCO_VERSION}-linux-x86_64.tar.gz
    exit 1
fi

# Remove the archive file
echo "Cleaning up temporary files..."
rm mujoco${MUJOCO_VERSION}-linux-x86_64.tar.gz

# Move MuJoCo to the lib directory
echo "Moving MuJoCo to the lib directory..."
if [ ! -d "$LIB_DIR/mujoco${MUJOCO_VERSION}" ]; then
    mkdir -p "$LIB_DIR/mujoco${MUJOCO_VERSION}"
fi
mv mujoco210 "mujoco"

# Verify that MuJoCo was installed correctly
echo "Verifying MuJoCo installation..."
if [ -d "$LIB_DIR/mujoco" ]; then
    echo "MuJoCo has been successfully installed in $LIB_DIR/mujoco/."
else
    echo "Error: MuJoCo directory does not exist."
    exit 1
fi

echo "You can now configure your CMake project."
