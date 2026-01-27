# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a collection of open-source audio plugins written in C++, built with JUCE 8. Legacy VST2 source code is preserved in the `legacy/` directory.

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

### Testing JUCE Plugins
Use the VST3 Plugin Test Host:
```bash
/Applications/VST3PluginTestHost.app/Contents/MacOS/VST3PluginTestHost \
    --pluginfolder "<pluginname>/build/<PluginName>_artefacts/VST3"
```

## Licensing
Dual-licensed: GPL for open source, commercial licensing available. Contributors must sign a CLA.

---

## JUCE Migration Project

### Goal
Convert all plugins from the legacy VST2.x SDK + VSTGUI framework to JUCE 8.0.12. The original VST2.x SDK and VSTGUI version used here are obsolete and no longer maintained.

### Approach
- **Preserve DSP code**: The audio processing/DSP classes must remain unchanged. Only the plugin infrastructure and UI code should be rewritten.
- **One plugin at a time**: Convert each plugin individually.
- **Plugin directories**: Each plugin lives in a lowercase directory (e.g., `h2o/`).
- **Independent builds**: Each plugin has its own standalone CMakeLists.txt, also buildable from the root CMakeLists.txt.

---

## Migration Template

### Directory Structure
```
<pluginname>/
├── CMakeLists.txt
├── Resources/
│   └── (copy all .png images from original plugin)
└── Source/
    ├── PluginProcessor.cpp
    ├── PluginProcessor.h
    ├── PluginEditor.cpp       # only for GUI plugins
    ├── PluginEditor.h
    ├── DSP/
    │   └── (copy DSP files unchanged from original)
    └── UI/
        └── (copy from existing plugins as needed)
```

### CMakeLists.txt Template
```cmake
cmake_minimum_required(VERSION 3.22)
project(<PluginName> VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(<PluginName>
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE <4CharCode>
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "<PluginName>"
)

# Binary resources (images) - only for GUI plugins
juce_add_binary_data(<PluginName>_Assets SOURCES
    Resources/back.png
    # ... list all PNG files
)

target_sources(<PluginName> PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp        # omit for no-GUI plugins
    Source/DSP/<DSPFile>.cpp
    Source/UI/FilmStripKnob.cpp    # omit for no-GUI plugins
)

target_compile_definitions(<PluginName>
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(<PluginName>
    PRIVATE
        <PluginName>_Assets         # omit for no-GUI plugins
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

### PluginProcessor Mapping

| Legacy (VST2) | JUCE |
|---------------|------|
| `AudioEffectX` base class | `juce::AudioProcessor` |
| `processReplacing()` | `processBlock()` |
| `setParameter()`/`getParameter()` | `juce::AudioProcessorValueTreeState` (APVTS) |
| `setSampleRate()` | `prepareToPlay()` |

Key patterns:
- Use APVTS for all parameters with `getRawParameterValue()` for thread-safe access
- **Always call DSP setters every `processBlock()`** - don't optimize with "isDirty" flags
- Implement `getStateInformation()`/`setStateInformation()` for preset saving

### PluginEditor Mapping

**For plugins WITHOUT a custom GUI** (like Bouncy, CrazyIvan, SupaTrigga, OnePingOnly):
- Return `false` from `hasEditor()`
- Return `new juce::GenericAudioProcessorEditor(*this)` from `createEditor()`
- Skip creating PluginEditor.cpp/h entirely

**For plugins WITH a custom GUI** (like H2O, AnechoicRoomSimulator):

| Legacy (VSTGUI) | JUCE |
|-----------------|------|
| `AEffGUIEditor` base class | `juce::AudioProcessorEditor` |
| `CBitmap` | `juce::Image` via `juce::ImageCache::getFromMemory()` |
| `CAnimKnob` | `FilmStripKnob` (custom component) |
| `COnOffButton` | `FilmStripKnob` with `isToggle=true` |
| `CTextDisplay` | `BitmapTextDisplay` or `TextDisplay` (see note below) |
| `CSplashScreen` | `SplashOverlay` (custom component) |

### Reusable UI Components

Copy from existing plugins as needed:

1. **FilmStripKnob** (`h2o/Source/UI/`) - Renders knobs from vertical film strip images
   - Supports both continuous knobs and toggle buttons
   - `FilmStripKnobAttachment` connects to APVTS

2. **BitmapTextDisplay** (`h2o/Source/UI/`) - Renders text using bitmap font with ASCII-indexed character widths

3. **TextDisplay** (`anechoicroomsimulator/Source/UI/`) - Alternative bitmap font renderer using character INDEX mapping (0-53) rather than ASCII codes. Check the original plugin's `asciitable.cpp` to determine which format to use.

4. **SplashOverlay** (`h2o/Source/UI/`) - Full-screen splash/about overlay, click to dismiss

5. **ClickArea** (`anechoicroomsimulator/Source/UI/`) - Simple invisible clickable area for trigger zones

6. **FilmStripSlider** (`supaphaser/Source/UI/`) - Vertical slider with a film strip handle image, supports handle x-offset

7. **MultiStateButton** (`supaphaser/Source/UI/`) - Cycles through N discrete states on click, shows different film strip frames per state

---

## Common Issues and Solutions

### VST3 shows extra MIDI CC parameters
**Solution**: Add `JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0` to compile definitions.

### Parameters don't affect the sound
**Solution**: Call DSP setters every `processBlock()`, not just when "dirty".

### Plugin crashes on load
**Solution**: Initialize DSP objects in constructor with default sample rate (44100), update in `prepareToPlay()`.

### Randomise button doesn't update host UI
**Solution**: Use `AsyncUpdater` to call `setValueNotifyingHost()` from the message thread:
```cpp
class MyProcessor : public juce::AudioProcessor, private juce::AsyncUpdater {
    std::atomic<bool> pendingRandomise { false };

    void processBlock(...) {
        if (randomiseTriggered) {
            pendingRandomise.store(true);
            triggerAsyncUpdate();
        }
    }

    void handleAsyncUpdate() override {
        if (pendingRandomise.exchange(false)) {
            for (auto* param : getParameters())
                param->setValueNotifyingHost(juce::Random::getSystemRandom().nextFloat());
        }
    }
};
```

### Parameter values exceed expected range after randomization
**Solution**: Use `juce::Random` instead of `rand()`. The `rand()` function returns values up to `RAND_MAX` which may be much larger than expected (2^31-1 on some systems).

### Parameter displays show raw 0-1 values
**Solution**: Use `AudioParameterFloat` with custom string conversion:
```cpp
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID(PARAM_ID, 1), "Name",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
    defaultValue, juce::String(),
    juce::AudioProcessorParameter::genericParameter,
    [](float v, int) { return juce::String(static_cast<int>(v * 100)) + "%"; },
    nullptr));
```

### Host tempo sync
```cpp
float bpm = 120.0f;
if (auto* playHead = getPlayHead()) {
    if (auto posInfo = playHead->getPosition()) {
        if (auto tempo = posInfo->getBpm())
            bpm = static_cast<float>(*tempo);
    }
}
```

---

## Completed Migrations

### H2O (Compressor)
- **Location**: `h2o/`
- **Has GUI**: Yes (FilmStripKnob, BitmapTextDisplay, SplashOverlay)
- **Parameters**: preamp, attack, release, amount, postamp, saturate

### Bouncy (Bouncing Ball Delay)
- **Location**: `bouncy/`
- **Has GUI**: No (GenericAudioProcessorEditor)
- **Parameters**: maxDelay, delayShape, ampShape, randAmp, renewRand
- **Special**: Host tempo sync, MIDI CC control (CC 73-77)

### CrazyIvan (Feedback Distortion)
- **Location**: `crazyivan/`
- **Has GUI**: No (GenericAudioProcessorEditor)
- **Parameters**: 22 parameters including randomise button
- **Special**: Randomise button with AsyncUpdater for UI sync

### SupaTrigga (Beat Slicer)
- **Location**: `supatrigga/`
- **Has GUI**: No (GenericAudioProcessorEditor)
- **Parameters**: granularity, speed, probReverse, probSpeed, probRearrange, probSilence, probRepeat, instantReverse, instantSpeed, instantRepeat
- **Special**: Host transport required (tempo, time signature, bar position)

### OnePingOnly (Ping Synthesizer)
- **Location**: `onepingonly/`
- **Has GUI**: No (GenericAudioProcessorEditor)
- **Parameters**: 771 total (128 pings × 6 params + 3 global)
- **Special**: IS_SYNTH TRUE, NEEDS_MIDI_INPUT TRUE, 128 resonant filter oscillators

### AnechoicRoomSimulator (April Fools)
- **Location**: `anechoicroomsimulator/`
- **Has GUI**: Yes (TextDisplay, FilmStripKnob, SplashOverlay, ClickArea)
- **Parameters**: size (does nothing - it's a joke!)
- **Special**: Custom TextDisplay using character index mapping (different from H2O's BitmapTextDisplay)

### SupaPhaser (Deep Phaser)
- **Location**: `supaphaser/`
- **Has GUI**: Yes (FilmStripKnob, FilmStripSlider, MultiStateButton, TextDisplay, SplashOverlay, ClickArea)
- **Parameters**: attack, release, minEnv, maxEnv, mixture, freq, minFreq, maxFreq, extend, stereo, stages, distort, feedback, dryWet, gain, invert
- **Special**: Envelope follower + LFO modulation, WavetableFPOsc for LFO, distortion, 4-state invert button, inverse bitmap knob for mixture, donation link in splash overlay

---

## Plugins To Migrate

1. **Smexoscope** - Oscilloscope with retrigger modes
2. **Bitmurderer** - Byte manipulation effect
