cmake_minimum_required(VERSION 3.22)
project(SupaPhaser VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE (skipped when building from root CMakeLists.txt)
if(NOT TARGET juce::juce_core)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Add the plugin
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

# Add binary resources (images)
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

# Source files
target_sources(SupaPhaser PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/WavetableFPOsc.cpp
    Source/UI/FilmStripKnob.cpp
    Source/UI/FilmStripSlider.cpp
    Source/UI/TextDisplay.cpp
    Source/UI/SplashOverlay.cpp
)

target_compile_definitions(SupaPhaser
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(SupaPhaser
    PRIVATE
        SupaPhaser_Assets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
