#!/bin/bash

# Get the directory where the script is located (in WSL format)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Convert the config.xlaunch path to Windows format
CONFIG_XLAUNCH_WIN_PATH=$(wslpath -w "$SCRIPT_DIR/config.xlaunch")

# Function to check if VcXsrv is already running
is_vcxsrv_running() {
    # Use Windows tasklist to check for vcxsrv.exe
    tasklist.exe | grep -i "vcxsrv.exe" > /dev/null
    return $?
}

# Check if the config.xlaunch file exists
if [ -f "$SCRIPT_DIR/config.xlaunch" ]; then
    if is_vcxsrv_running; then
        echo "VcXsrv is already running. Skipping launch."
    else
        echo "Launching VcXsrv with config.xlaunch..."
        # Run the .xlaunch file with VcXsrv (Windows path)
        cmd.exe /c "$CONFIG_XLAUNCH_WIN_PATH"
    fi
else
    echo "Error: config.xlaunch not found in the script's directory."
    exit 1
fi

# Export the DISPLAY variable
export DISPLAY=:0
echo "DISPLAY set to :0 with export DISPLAY=:0"

# Final message
echo "X server check complete and DISPLAY is set."
