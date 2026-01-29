cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(OnePingOnly VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(OnePingOnly
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE Ping
    LV2URI "https://familiedejong.be/plugins/OnePingOnly"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "One Ping Only"
    PLUGIN_CHANNEL_CONFIGURATIONS "{0, 2}"
)

target_sources(OnePingOnly PRIVATE
    Source/PluginProcessor.cpp
    Source/Delay.cpp
)

smartelectronix_plugin_common(OnePingOnly)
