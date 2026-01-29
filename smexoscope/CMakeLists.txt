cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(Smexoscope VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(Smexoscope
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE osX2
    LV2URI "https://familiedejong.be/plugins/Smexoscope"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Smexoscope"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(Smexoscope_Assets SOURCES
    Resources/blue_knob1_4.png
    Resources/body.png
    Resources/free_etc.png
    Resources/heads.png
    Resources/lefr_right.png
    Resources/off_on.png
    Resources/readout.png
    Resources/slider_new.png
    Resources/dark/blue_knob1_4.png
    Resources/dark/body.png
    Resources/dark/free_etc.png
    Resources/dark/heads.png
    Resources/dark/lefr_right.png
    Resources/dark/off_on.png
    Resources/dark/readout.png
    Resources/dark/slider_new.png
)

target_sources(Smexoscope PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/FilmStripSlider.cpp
    Source/UI/WaveDisplay.cpp
)

smartelectronix_plugin_common(Smexoscope)
target_link_libraries(Smexoscope PRIVATE Smexoscope_Assets)
