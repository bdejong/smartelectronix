cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(SupaPhaser VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(SupaPhaser
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE ASuP
    LV2URI "https://familiedejong.be/plugins/SupaPhaser"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "SupaPhaser"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(SupaPhaser_Assets SOURCES
    Resources/base.png
    Resources/blue_knob1_4.png
    Resources/bigknob01rotated_cropped2.png
    Resources/small_knob02.png
    Resources/type_grey-back_white.png
    Resources/type_white-back.png
    Resources/type_orange-back.png
    Resources/extend_on_off.png
    Resources/slider.png
    Resources/heads.png
    Resources/splash.png
)

target_sources(SupaPhaser PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/WavetableFPOsc.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/FilmStripSlider.cpp
    Source/UI/TextDisplay.cpp
    Source/UI/SplashOverlay.cpp
)

smartelectronix_plugin_common(SupaPhaser)
target_link_libraries(SupaPhaser PRIVATE SupaPhaser_Assets)
