cmake_minimum_required(VERSION 3.22)

# Include common settings for standalone builds
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)
endif()

project(Bouncy VERSION 1.0.0)

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
juce_add_plugin(Bouncy
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE Bncy
    LV2URI "https://familiedejong.be/plugins/Bouncy"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Bouncy"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

# Source files
target_sources(Bouncy PRIVATE
    Source/PluginProcessor.cpp
    Source/DSP/Multitap.cpp
)

target_compile_definitions(Bouncy
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(Bouncy
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
