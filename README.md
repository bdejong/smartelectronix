# Bram @ Smartelectronix

Open source versions of all bram @ smartelectronix plugins. 20 Years after working on my first plugin I've decided to open source all these plugins. Consider a lot of this source code was written by a very inexperienced version of myself: it's kind of a mess!

You can find the original smartelectronix pages here: http://bram.smartelectronix.com

## Downloads

Pre-built plugins for macOS, Windows, and Linux are available on the [Releases page](https://github.com/bdejong/smartelectronix/releases).

## Donations

http://paypal.me/BramdeJong

## Plugin list

- **AnechoicRoomSimulator**: Silly 1st of April plugin
- **Bitmurderer**: Unreleased plugin which can x-or and mess up bytes in the incoming audio. Even has a nice GUI, but was never finished...
- **Bouncy**: Bouncing ball delay.
- **CrazyIvan**: Insane feedback with distortion effect.
- **Cyanide2**: Spline wave-shaper with oversampling.
- **H2O**: Heavy pumping compressor.
- **Madshifta**: Strange pitch-shifting and delay effect. A collaboration between me (original algorithm), TobyBear.de (translation to Delphi & UI), Sophia Poirier (translation to C++ and AU).
- **OnePingOnly**: Simple ping-generating synth.
- **S(m)exoscope**: Oscilloscope plugin that lets you retrigger the oscilloscope in a few different ways.
- **SupaPhaser2**: Deep phaser.
- **SupaTrigga**: Tempo-locked stuttering effect.

## Building

All plugins use [JUCE 8](https://juce.com/) and build with CMake. JUCE is fetched automatically during configuration.

### Prerequisites

- CMake 3.22+
- **macOS**: Xcode
- **Windows**: Visual Studio 2019+

### Build all plugins at once

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Built plugins are in `build/<pluginname>/<PluginName>_artefacts/` with subdirectories for each format (Standalone, AU, VST3).

### Build a single plugin

```bash
cd <pluginname>
cmake -B build
cmake --build build
```

## Commercial non-GPL licensing

All these plugins are available for licensing under a dual-license scheme. GPL for open source and non-GPL for commercial usage. For those people who want to collaborate on the project there is a [CLA](https://github.com/bdejong/smartelectronix/wiki/CLA).

## Credits

These plugins were converted from the legacy VST2 SDK to JUCE with help from [Claude Code](https://claude.ai/code).
