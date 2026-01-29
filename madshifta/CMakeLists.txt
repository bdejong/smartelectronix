cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(Madshifta VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(Madshifta
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE TBMS
    LV2URI "https://familiedejong.be/plugins/Madshifta"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "MadShifta"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(Madshifta_Assets SOURCES
    Resources/background.png
    Resources/filter_mode_button.png
    Resources/midi_mode_button.png
    Resources/info.png
)

target_sources(Madshifta PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/SplashOverlay.cpp
)

smartelectronix_plugin_common(Madshifta)
target_link_libraries(Madshifta PRIVATE Madshifta_Assets)
