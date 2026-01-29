cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CommonSettings.cmake)

project(SmartelectronixPlugins VERSION 1.0.0)

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# Fetch JUCE once for all plugins
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Add all plugins
add_subdirectory(anechoicroomsimulator)
add_subdirectory(bitmurderer)
add_subdirectory(bouncy)
add_subdirectory(crazyivan)
add_subdirectory(cyanide2)
add_subdirectory(h2o)
add_subdirectory(madshifta)
add_subdirectory(onepingonly)
add_subdirectory(smexoscope)
add_subdirectory(supaphaser)
add_subdirectory(supatrigga)

