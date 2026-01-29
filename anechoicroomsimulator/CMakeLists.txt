cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(AnechoicRoomSimulator VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(AnechoicRoomSimulator
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE AneC
    LV2URI "https://familiedejong.be/plugins/AnechoicRoomSimulator"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Anechoic Room Simulator"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(AnechoicRoomSimulator_Assets SOURCES
    Resources/back.png
    Resources/knob.png
    Resources/type.png
    Resources/splash.png
)

target_sources(AnechoicRoomSimulator PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/TextDisplay.cpp
    Source/UI/SplashOverlay.cpp
)

smartelectronix_plugin_common(AnechoicRoomSimulator)
target_link_libraries(AnechoicRoomSimulator PRIVATE AnechoicRoomSimulator_Assets)
