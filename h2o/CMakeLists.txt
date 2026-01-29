cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(H2O VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(H2O
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE H2Oj
    LV2URI "https://familiedejong.be/plugins/H2O"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "H2O"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(H2O_Assets SOURCES
    Resources/ascii.png
    Resources/back.png
    Resources/knob.png
    Resources/saturate.png
    Resources/splash.png
)

target_sources(H2O PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/Compressor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/BitmapTextDisplay.cpp
    Source/UI/SplashOverlay.cpp
)

smartelectronix_plugin_common(H2O)
target_link_libraries(H2O PRIVATE H2O_Assets)
