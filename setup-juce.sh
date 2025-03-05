#!/bin/bash

# Exit on error
set -e

# Add JUCE as a submodule
if [ ! -d "JUCE" ]; then
    echo "Adding JUCE as a submodule..."
    git submodule add https://github.com/juce-framework/JUCE.git
    git submodule update --init --recursive
fi

echo "JUCE setup complete. The submodule is now available for all plugins."
echo "To build the OnePingOnly-JUCE project, run:"
echo
echo "mkdir -p OnePingOnly-JUCE/build"
echo "cd OnePingOnly-JUCE/build"
echo "cmake .."
echo "cmake --build ."