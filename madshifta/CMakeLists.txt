cmake_minimum_required(VERSION 3.22)
project(Madshifta VERSION 1.0.0)

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

# Add the plugin
juce_add_plugin(Madshifta
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE TBMS
    LV2URI "https://familiedejong.be/plugins/Madshifta"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "MadShifta"
)

# Add binary resources (images)
juce_add_binary_data(Madshifta_Assets SOURCES
    Resources/background.png
    Resources/filter_mode_button.png
    Resources/midi_mode_button.png
    Resources/info.png
)

# Source files
target_sources(Madshifta PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/SplashOverlay.cpp
)

target_compile_definitions(Madshifta
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(Madshifta
    PRIVATE
        Madshifta_Assets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
