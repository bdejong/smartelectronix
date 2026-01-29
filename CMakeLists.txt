cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CommonSettings.cmake)

project(SmartelectronixPlugins VERSION 1.0.0)

smartelectronix_fetch_juce()

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
