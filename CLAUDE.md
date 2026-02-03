# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A collection of open-source audio plugins written in C++, built with JUCE 8.0.12. Originally VST2/VSTGUI plugins, now converted to modern JUCE supporting AU, VST3, LV2, and Standalone formats.

## Build Commands

### Build all plugins from root
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

### Build a single plugin standalone
```bash
cd <pluginname>
cmake -B build
cmake --build build -j
```

Built plugins are in `build/<PluginName>_artefacts/{VST3,AU,LV2,Standalone}/`

### Testing
```bash
# macOS VST3 test host
/Applications/VST3PluginTestHost.app/Contents/MacOS/VST3PluginTestHost \
    --pluginfolder "<pluginname>/build/<PluginName>_artefacts/VST3"
```

Prerequisites: CMake 3.22+, Xcode (macOS), Visual Studio 2019+ (Windows), or GCC/Clang (Linux with ALSA/X11 dev packages)

---

## CMake Architecture

### Shared Settings: `cmake/CommonSettings.cmake`

All build settings are centralized in `cmake/CommonSettings.cmake`:

```cmake
# Included BEFORE project() in each CMakeLists.txt
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Symbol visibility (reduces binary size, prevents symbol collisions)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
```

### Shared Macros/Functions

**`smartelectronix_fetch_juce()`** - Fetches JUCE 8.0.12 if not already available (call after `project()`):
```cmake
smartelectronix_fetch_juce()
```

**`smartelectronix_plugin_common(target_name)`** - Applies common compile definitions and link libraries:
```cmake
smartelectronix_plugin_common(MyPlugin)
target_link_libraries(MyPlugin PRIVATE MyPlugin_Assets)  # If plugin has GUI
```

### Disabled JUCE Features

The following are disabled to speed up builds (in `smartelectronix_plugin_common`):

```cmake
# Unused features
JUCE_WEB_BROWSER=0
JUCE_USE_CURL=0
JUCE_USE_CAMERA=0
JUCE_USE_CDREADER=0
JUCE_USE_CDBURNER=0

# Audio formats not needed
JUCE_USE_OGGVORBIS=0
JUCE_USE_MP3AUDIOFORMAT=0
JUCE_USE_FLAC=0
JUCE_USE_LAME_AUDIO_FORMAT=0

# Plugin hosting (for DAWs, not plugins)
JUCE_PLUGINHOST_VST=0
JUCE_PLUGINHOST_VST3=0
JUCE_PLUGINHOST_AU=0
JUCE_PLUGINHOST_LADSPA=0
JUCE_PLUGINHOST_LV2=0

# VST3 settings
JUCE_VST3_CAN_REPLACE_VST2=0      # No VST2 to replace
JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0  # Prevents extra MIDI CC parameters
```

### Plugin CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(MyPlugin VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(MyPlugin
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE MyPl                    # Unique 4-char code
    LV2URI "https://familiedejong.be/plugins/MyPlugin"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "My Plugin"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect  # or kAudioUnitType_MusicEffect for MIDI
)

# Binary resources (GUI plugins only)
juce_add_binary_data(MyPlugin_Assets SOURCES
    Resources/background.png
    Resources/knob.png
)

target_sources(MyPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp           # GUI plugins only
    Source/DSP/MyDSP.cpp
    Source/UI/FilmStripKnob.cpp       # GUI plugins only
)

smartelectronix_plugin_common(MyPlugin)
target_link_libraries(MyPlugin PRIVATE MyPlugin_Assets)  # GUI plugins only
```

---

## Plugin Architecture

### Directory Structure
```
<pluginname>/
├── CMakeLists.txt
├── Resources/              # PNG images for GUI
│   ├── background.png
│   ├── knob.png
│   └── dark/              # Optional dark skin (Smexoscope)
└── Source/
    ├── PluginProcessor.cpp/h   # Audio processing, parameters, state
    ├── PluginEditor.cpp/h      # GUI (optional)
    ├── DSP/                    # DSP algorithms (preserved from legacy)
    └── UI/                     # Custom JUCE components
```

### PluginProcessor Pattern

```cpp
class MyProcessor : public juce::AudioProcessor {
public:
    MyProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }  // false for no-GUI plugins

    juce::AudioProcessorValueTreeState apvts;

    // Parameter IDs as static constexpr
    static constexpr const char* PARAM_GAIN = "gain";

private:
    std::atomic<float>* gainParam = nullptr;  // Raw parameter pointers

    // DSP objects
    MyDSP dsp;
};
```

### Parameter Best Practices

1. **Use APVTS** for all parameters with `getRawParameterValue()` for thread-safe access
2. **Call DSP setters every processBlock()** - don't optimize with dirty flags
3. **Custom string formatting** for display:
```cpp
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID("gain", 1), "Gain",
    juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
    0.5f, juce::String(),
    juce::AudioProcessorParameter::genericParameter,
    [](float v, int) { return juce::String(v * 100, 1) + "%"; },
    nullptr));
```

### State Saving with Extra Properties

To save non-parameter state (like UI preferences):
```cpp
void MyProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    state.setProperty("myProperty", myValue, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MyProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType())) {
        auto state = juce::ValueTree::fromXml(*xml);
        myValue = static_cast<bool>(state.getProperty("myProperty", defaultValue));
        apvts.replaceState(state);
    }
}
```

### Host Tempo Sync

```cpp
void MyProcessor::processBlock(...) {
    float bpm = 120.0f;
    if (auto* playHead = getPlayHead()) {
        if (auto posInfo = playHead->getPosition()) {
            if (auto tempo = posInfo->getBpm())
                bpm = static_cast<float>(*tempo);
        }
    }
}
```

---

## Reusable UI Components

Copy from existing plugins as needed:

| Component | Location | Description |
|-----------|----------|-------------|
| FilmStripKnob | `h2o/Source/UI/` | Knobs/toggles from vertical film strip images |
| FilmStripSlider | `supaphaser/Source/UI/` | Vertical slider with film strip handle |
| MultiStateButton | `supaphaser/Source/UI/` | Cycles through N discrete states on click |
| BitmapTextDisplay | `h2o/Source/UI/` | Bitmap font with ASCII-indexed character widths |
| TextDisplay | `anechoicroomsimulator/Source/UI/` | Bitmap font with character index mapping (0-53) |
| SplashOverlay | `h2o/Source/UI/` | Full-screen splash/about overlay, click to dismiss |
| ClickArea | `anechoicroomsimulator/Source/UI/` | Invisible clickable trigger zone |
| WaveDisplay | `smexoscope/Source/UI/` | Oscilloscope waveform display |

### FilmStripKnob Usage

```cpp
// In constructor
knob = std::make_unique<FilmStripKnob>(knobImage, numFrames);
knob->setBounds(x, y, width, frameHeight);
addAndMakeVisible(*knob);

// Attachment
knobAttachment = std::make_unique<FilmStripKnobAttachment>(
    processor.apvts, "paramId", *knob);

// For toggle buttons
toggle = std::make_unique<FilmStripKnob>(onOffImage, 2, true);  // isToggle=true
```

### Skin Switching (Smexoscope example)

```cpp
// In PluginProcessor.h
bool useDarkSkin = false;  // Saved with state

// In PluginProcessor constructor
useDarkSkin = juce::Desktop::getInstance().isDarkModeActive();

// In PluginEditor - load both skins
lightSkin.background = juce::ImageCache::getFromMemory(BinaryData::body_png, ...);
darkSkin.background = juce::ImageCache::getFromMemory(BinaryData::body_png2, ...);

// Toggle on keypress
bool keyPressed(const juce::KeyPress&) override {
    useDarkSkin = !useDarkSkin;
    processorRef.useDarkSkin = useDarkSkin;
    applySkin(useDarkSkin);
    return true;
}

// Apply skin to all components
void applySkin(bool dark) {
    auto& skin = dark ? darkSkin : lightSkin;
    backgroundImage = skin.background;
    knob->setImage(skin.knob);
    // ... update all components
    repaint();
}
```

**Note**: JUCE names binary resources from subdirectories with numeric suffixes (e.g., `dark/body.png` becomes `body_png2`).

---

## Plugin List

| Plugin | Directory | GUI | Notes |
|--------|-----------|-----|-------|
| AnechoicRoomSimulator | `anechoicroomsimulator/` | Yes | April Fools joke plugin |
| Bitmurderer | `bitmurderer/` | Yes | Byte manipulation effect |
| Bouncy | `bouncy/` | No | Bouncing ball delay, tempo sync, MIDI CC 73-77 |
| CrazyIvan | `crazyivan/` | No | Feedback distortion, 22 params, randomise |
| Cyanide2 | `cyanide2/` | Yes | Spline wave-shaper with 4x oversampling |
| H2O | `h2o/` | Yes | Compressor |
| Madshifta | `madshifta/` | Yes | Pitch-shifting and delay |
| OnePingOnly | `onepingonly/` | No | Synth (128 resonant filters), IS_SYNTH=TRUE |
| Smexoscope | `smexoscope/` | Yes | Oscilloscope, tempo sync, dark skin toggle |
| SupaPhaser | `supaphaser/` | Yes | Deep phaser, envelope follower + LFO |
| SupaTrigga | `supatrigga/` | No | Tempo-locked stuttering, host transport |

### Plugin-Specific Notes

**OnePingOnly**: Set `IS_SYNTH TRUE` and `NEEDS_MIDI_INPUT TRUE` in CMakeLists.txt, use `PLUGIN_CHANNEL_CONFIGURATIONS "{0, 2}"` for no input.

**Smexoscope**: Has dark/light skin toggle. Press any key to switch. Defaults to OS dark mode setting. Skin preference is saved with plugin state.

**Bouncy/SupaTrigga**: Require host transport (tempo, time signature, playhead position).

---

## CI/CD: GitHub Actions

`.github/workflows/build.yml` builds for macOS, Windows, and Linux:

- Triggers on push to master, PRs, and manual dispatch
- Builds with `--parallel 2` to avoid memory issues
- Creates rolling release on master pushes (tag: `latest`)
- Artifacts: VST3, AU (macOS), LV2, Standalone

---

## Common Issues and Solutions

### VST3 shows extra MIDI CC parameters
Add `JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0` (already in CommonSettings).

### Parameters don't affect sound
Call DSP setters every `processBlock()`, not just when "dirty".

### Plugin crashes on load
Initialize DSP objects in constructor with default sample rate (44100), update in `prepareToPlay()`.

### Randomise button doesn't update host UI
Use `AsyncUpdater` to call `setValueNotifyingHost()` from the message thread.

### "App is damaged" on macOS
Run: `xattr -cr /path/to/plugin`

### Binary resource naming with subdirectories
Files in subdirectories get numeric suffixes: `dark/body.png` → `body_png2`, `body_png2Size`

---

## Licensing

Dual-licensed: GPL for open source, commercial licensing available. Contributors must sign a CLA.
