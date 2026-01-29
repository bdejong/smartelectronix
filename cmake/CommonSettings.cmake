# CommonSettings.cmake - Shared settings for all Smartelectronix plugins
# Include this BEFORE project() in each plugin's CMakeLists.txt

# macOS deployment target (must be before project())
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Symbol visibility settings for plugins
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Fetch JUCE (call after project(), skipped when juce already available)
macro(smartelectronix_fetch_juce)
    if(NOT TARGET juce::juce_core)
        include(FetchContent)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.12
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(JUCE)
    endif()
endmacro()

# Apply common compile definitions and link libraries to a plugin target
function(smartelectronix_plugin_common target_name)
    target_compile_definitions(${target_name} PUBLIC
        # Disable unused JUCE features
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_CAMERA=0
        JUCE_USE_CDREADER=0
        JUCE_USE_CDBURNER=0
        # Disable audio formats not needed
        JUCE_USE_OGGVORBIS=0
        JUCE_USE_MP3AUDIOFORMAT=0
        JUCE_USE_FLAC=0
        JUCE_USE_LAME_AUDIO_FORMAT=0
        # Disable plugin hosting (these are for DAWs, not plugins)
        JUCE_PLUGINHOST_VST=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_PLUGINHOST_AU=0
        JUCE_PLUGINHOST_LADSPA=0
        JUCE_PLUGINHOST_LV2=0
        # VST3 settings
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=0
    )
    target_link_libraries(${target_name}
        PRIVATE
            juce::juce_audio_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endfunction()
