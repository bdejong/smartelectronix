cmake_minimum_required(VERSION 3.22)
project(Bitmurderer VERSION 1.0.0)

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

juce_add_plugin(Bitmurderer
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE BITM
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Bitmurderer"
)

juce_add_binary_data(Bitmurderer_Assets SOURCES
    Resources/bitmurderer.png
    Resources/1.png
    Resources/2.png
    Resources/credits.png
)

target_sources(Bitmurderer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/SplashOverlay.cpp
)

target_compile_definitions(Bitmurderer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(Bitmurderer
    PRIVATE
        Bitmurderer_Assets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
