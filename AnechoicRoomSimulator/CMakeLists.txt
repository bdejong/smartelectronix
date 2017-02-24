cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(AnechoicRoomSimulator)

set(PROJECT_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaEditor.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AnechoicRoomSim.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AnechoicRoomSim.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/AnechoicRoomSimMain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.hpp
)

set(PROJECT_IMAGES
  ${CMAKE_CURRENT_SOURCE_DIR}/images/back.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/knob.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/splash.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/type.png
)

build_vst_gui(AnechoicRoomSimulator "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
