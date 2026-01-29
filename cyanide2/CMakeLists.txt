cmake_minimum_required(VERSION 3.22)
project(Cyanide2 VERSION 1.0.0)

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

# Add binary resources (images)
juce_add_binary_data(Cyanide2_Assets SOURCES
    Resources/background.png
    Resources/knob_handle.png
    Resources/shaper_handle.png
    Resources/switch.png
    Resources/guides.png
    Resources/reset.png
    Resources/splash.png
)

# Source files
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

target_compile_definitions(Cyanide2
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
)

target_link_libraries(Cyanide2
    PRIVATE
        Cyanide2_Assets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
