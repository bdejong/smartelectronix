cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake)

project(Cyanide2 VERSION 1.0.0)

smartelectronix_fetch_juce()

juce_add_plugin(Cyanide2
    COMPANY_NAME "Smartelectronix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Smex
    PLUGIN_CODE Cya2
    LV2URI "https://familiedejong.be/plugins/Cyanide2"
    FORMATS AU VST3 LV2 Standalone
    PRODUCT_NAME "Cyanide2"
    PLUGIN_CHANNEL_CONFIGURATIONS "{2, 2}"
    AU_MAIN_TYPE kAudioUnitType_Effect
)

juce_add_binary_data(Cyanide2_Assets SOURCES
    Resources/background.png
    Resources/knob_handle.png
    Resources/shaper_handle.png
    Resources/switch.png
    Resources/guides.png
    Resources/reset.png
    Resources/splash.png
)

target_sources(Cyanide2 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/Shaper.cpp
    Source/DSP/Spline.cpp
    Source/DSP/ButterXOver.cpp
    Source/DSP/Polyphase.cpp
    Source/UI/RotaryKnob.cpp
    Source/UI/ShaperViewComponent.cpp
    Source/UI/PreviewComponent.cpp
    Source/UI/SplashOverlay.cpp
    Source/UI/FilmStripKnob.cpp
)

smartelectronix_plugin_common(Cyanide2)
target_link_libraries(Cyanide2 PRIVATE Cyanide2_Assets)
