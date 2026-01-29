cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(CrazyIvan VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(CrazyIvan
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE Crzy
    LV2URI "https://familiedejong.be/plugins/CrazyIvan"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Crazy Ivan"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

target_sources(CrazyIvan PRIVATE
    Source/PluginProcessor.cpp
    Source/DSP/OnePoleLP.cpp
    Source/DSP/TriOsc.cpp
    Source/DSP/Flanger.cpp
    Source/DSP/EnvFollower.cpp
)

smartelectronix_plugin_common(CrazyIvan)
