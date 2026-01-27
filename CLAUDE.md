# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A collection of open-source audio plugins written in C++, built with JUCE 8. Legacy VST2 source code is preserved in the `legacy/` directory.

## Build Commands

### Build all plugins
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Prerequisites: CMake 3.22+, Xcode (macOS) or Visual Studio 2019+ (Windows)

### Build a single plugin
```bash
cd <pluginname>
cmake -B build
cmake --build build
```

### Testing
```bash
/Applications/VST3PluginTestHost.app/Contents/MacOS/VST3PluginTestHost \
    --pluginfolder "<pluginname>/build/<PluginName>_artefacts/VST3"
```

## Architecture

Each plugin follows the same structure:
```
<pluginname>/
├── CMakeLists.txt
├── Resources/          # PNG images (GUI plugins only)
└── Source/
    ├── PluginProcessor.cpp/h
    ├── PluginEditor.cpp/h   # GUI plugins only
    ├── DSP/                 # Audio processing (preserved from legacy code)
    └── UI/                  # Custom JUCE components
```

- Each plugin has its own `CMakeLists.txt` that fetches JUCE independently, guarded by `if(NOT TARGET juce::juce_core)` so the root CMakeLists.txt can share a single JUCE fetch.
- DSP code is preserved unchanged from the original plugins. Only infrastructure and UI were rewritten.
- All parameters use `AudioProcessorValueTreeState` (APVTS) with `getRawParameterValue()`.
- DSP setters are called every `processBlock()` (no dirty flags).

## Reusable UI Components

| Component | Location | Description |
|-----------|----------|-------------|
| FilmStripKnob | `h2o/Source/UI/` | Knobs/toggles from vertical film strip images |
| BitmapTextDisplay | `h2o/Source/UI/` | Bitmap font with ASCII-indexed character widths |
| TextDisplay | `anechoicroomsimulator/Source/UI/` | Bitmap font with character index mapping (0-53) |
| SplashOverlay | `h2o/Source/UI/` | Full-screen splash/about overlay |
| ClickArea | `anechoicroomsimulator/Source/UI/` | Invisible clickable trigger zone |
| FilmStripSlider | `supaphaser/Source/UI/` | Vertical slider with film strip handle |
| MultiStateButton | `supaphaser/Source/UI/` | Cycles through N discrete states on click |

## Plugin List

| Plugin | Directory | GUI | Notes |
|--------|-----------|-----|-------|
| AnechoicRoomSimulator | `anechoicroomsimulator/` | Yes | April Fools joke plugin |
| Bitmurderer | `bitmurderer/` | Yes | Byte manipulation effect |
| Bouncy | `bouncy/` | No | Bouncing ball delay, tempo sync, MIDI CC |
| CrazyIvan | `crazyivan/` | No | Feedback distortion, randomise button |
| Cyanide2 | `cyanide2/` | Yes | Spline wave-shaper with oversampling |
| H2O | `h2o/` | Yes | Compressor |
| Madshifta | `madshifta/` | Yes | Pitch-shifting and delay |
| OnePingOnly | `onepingonly/` | No | Synth (128 resonant filters), MIDI input |
| Smexoscope | `smexoscope/` | Yes | Oscilloscope with retrigger modes |
| SupaPhaser | `supaphaser/` | Yes | Deep phaser, envelope follower + LFO |
| SupaTrigga | `supatrigga/` | No | Tempo-locked stuttering, host transport |

## Licensing

Dual-licensed: GPL for open source, commercial licensing available. Contributors must sign a CLA.
