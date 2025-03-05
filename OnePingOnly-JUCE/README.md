# One Ping Only (JUCE Version)

This is a JUCE-based port of the original OnePingOnly VST plugin from Smartelectronix.

## Features

- 128 unique oscillators with configurable parameters
- Delay effect with adjustable feedback
- 12 factory presets
- MIDI note input triggering individual pings
- VST3 and AU plugin formats supported

## Building the Plugin

### Prerequisites

- CMake 3.15 or later
- A C++17 compatible compiler
- JUCE framework (added as a submodule in the parent directory)

### Setup

First, ensure JUCE is properly set up by running the setup script in the parent directory:

```bash
# From the root smartelectronix directory
./setup-juce.sh
```

### Building

```bash
# Create a build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build .
```

The built plugins will be in the `build` directory under platform-specific folders.

## Usage

- Each MIDI note (0-127) triggers a unique oscillator
- Use the global controls (delay time, feedback, and master volume) to shape the overall sound
- The program selector allows you to choose between 12 different presets

## Parameters

### Global Parameters

- **Delay Time**: Controls the delay time (0.0 to 1.0 seconds)
- **Feedback**: Controls the delay feedback (0.0 to 0.95)
- **Master Volume**: Controls the overall output volume

### Voice Parameters (Per MIDI Note)

Each MIDI note (0-127) has its own set of parameters:

- **Frequency**: Adjusts the oscillator frequency
- **Duration**: Controls the ping duration
- **Amplitude**: Sets the volume of the individual ping
- **Balance**: Controls stereo positioning
- **Noise**: Adds noise to the ping
- **Distortion**: Adds harmonic distortion to the ping

## License

This software is licensed under the same terms as the original Smartelectronix plugins.