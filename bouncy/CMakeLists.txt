cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(Bouncy VERSION 1.0.0)

smartelectronix_fetch_juce()

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

target_sources(Bouncy PRIVATE
    Source/PluginProcessor.cpp
    Source/DSP/Multitap.cpp
)

smartelectronix_plugin_common(Bouncy)
