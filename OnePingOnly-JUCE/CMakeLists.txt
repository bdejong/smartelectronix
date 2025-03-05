cmake_minimum_required(VERSION 3.15)

project(OnePingOnly VERSION 1.0.0)

# Include JUCE from the shared location in the parent directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE JUCE_build)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create binary data for assets
juce_add_binary_data(OnePingOnlyBinaryData
    SOURCES
        assets/logo.png
)

# Create plugin
juce_add_plugin(OnePingOnly
    VERSION 1.0.0
    COMPANY_NAME "Smartelectronix"
    PLUGIN_MANUFACTURER_CODE Smtx
    PLUGIN_CODE Ping
    FORMATS VST3 AU
    PRODUCT_NAME "One Ping Only"
    COMPANY_WEBSITE "https://github.com/bdejong/smartelectronix"
    COMPANY_EMAIL "info@smartelectronix.com"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    VST_NUM_MIDI_INPUTS 1
    VST_NUM_MIDI_OUTPUTS 0
)

# Source files
target_sources(OnePingOnly
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/Delay.cpp
)

# Include directories
target_include_directories(OnePingOnly
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# JUCE module dependencies
target_compile_definitions(OnePingOnly
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Required JUCE modules
juce_generate_juce_header(OnePingOnly)

target_link_libraries(OnePingOnly
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        OnePingOnlyBinaryData
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)