cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(SupaTrigga VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(SupaTrigga
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE GLTC
    LV2URI "https://familiedejong.be/plugins/SupaTrigga"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "SupaTrigga"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

target_sources(SupaTrigga PRIVATE
    Source/PluginProcessor.cpp
)

smartelectronix_plugin_common(SupaTrigga)
