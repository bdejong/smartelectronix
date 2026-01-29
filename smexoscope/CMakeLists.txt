cmake_minimum_required(VERSION 3.22)
project(Smexoscope VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE (skipped when building from root CMakeLists.txt)
if(NOT TARGET juce::juce_core)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Smexoscope
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE osX2
    LV2URI "https://familiedejong.be/plugins/Smexoscope"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Smexoscope"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(Smexoscope_Assets SOURCES
    Resources/blue_knob1_4.png
    Resources/body.png
    Resources/free_etc.png
    Resources/heads.png
    Resources/lefr_right.png
    Resources/off_on.png
    Resources/readout.png
    Resources/slider_new.png
)

target_sources(Smexoscope PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/FilmStripSlider.cpp
    Source/UI/WaveDisplay.cpp
)

target_compile_definitions(Smexoscope
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(Smexoscope
    PRIVATE
        Smexoscope_Assets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
