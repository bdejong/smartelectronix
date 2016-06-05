cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

project(Smexoscope)

set(PROJECT_SOURCE 
  ${CMAKE_CURRENT_SOURCE_DIR}/WaveDisplay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Label.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Label.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MultiStateButton.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/MultiStateButton.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SmartelectronixDisplay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SmartelectronixDisplay.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SmartelectronixDisplayEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SmartelectronixDisplayEditor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SmartelectronixDisplayMain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/WaveDisplay.cpp
)

set(PROJECT_IMAGES
  ${CMAKE_CURRENT_SOURCE_DIR}/images/blue_knob1_4.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/body.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/free_etc.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/heads.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/lefr_right.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/off_on.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/readout.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/slider_new.png
)

build_vst_gui(Smexoscope "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
