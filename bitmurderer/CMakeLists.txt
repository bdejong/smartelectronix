cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(Bitmurderer VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(Bitmurderer
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE BITM
    LV2URI "https://familiedejong.be/plugins/Bitmurderer"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Bitmurderer"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
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

smartelectronix_plugin_common(Bitmurderer)
target_link_libraries(Bitmurderer PRIVATE Bitmurderer_Assets)
