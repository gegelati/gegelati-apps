#!/bin/bash

# Update and install wget and unzip if necessary
echo "Updating system packages (requires sudo)..."
sudo apt update
sudo apt install -y wget unzip

# Working directory
LIB_DIR=$(pwd)

# MuJoCo version
MUJOCO_VERSION=210
MUJOCO_FOLDER="mujoco${MUJOCO_VERSION}"
MUJOCO_ARCHIVE="${MUJOCO_FOLDER}-linux-x86_64.tar.gz"
MUJOCO_URL="https://mujoco.org/download/${MUJOCO_ARCHIVE}"

# Download MuJoCo
echo "Downloading MuJoCo..."
wget -O "$MUJOCO_ARCHIVE" "$MUJOCO_URL"

if [ $? -ne 0 ]; then
    echo "Error: Failed to download MuJoCo."
    exit 1
fi

# Extract MuJoCo
echo "Extracting MuJoCo..."
tar -xzf "$MUJOCO_ARCHIVE"

if [ $? -ne 0 ]; then
    echo "Error: Failed to extract MuJoCo."
    rm "$MUJOCO_ARCHIVE"
    exit 1
fi

# Remove the archive
echo "Cleaning up archive..."
rm "$MUJOCO_ARCHIVE"

# Copy the folder instead of moving it (to avoid permission issues in /mnt/c)
echo "Copying MuJoCo to the lib directory (using cp -r)..."
mkdir -p "$LIB_DIR/mujoco"
cp -r "$MUJOCO_FOLDER"/* "$LIB_DIR/mujoco"

# Clean up the extracted folder
echo "Cleaning up temporary extracted folder..."
rm -rf "$MUJOCO_FOLDER"

# Verify installation
echo "Verifying MuJoCo installation..."
if [ -d "$LIB_DIR/mujoco/bin" ]; then
    echo "MuJoCo has been successfully installed in $LIB_DIR/mujoco/"
else
    echo "Error: MuJoCo directory or content missing."
    exit 1
fi

echo "Setup complete. You can now configure your CMake project."
