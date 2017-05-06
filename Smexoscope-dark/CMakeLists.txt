cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(Smexoscope-dark)

set(PROJECT_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/WaveDisplay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/Label.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/Label.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/MultiStateButton.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/MultiStateButton.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/SmartelectronixDisplay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/SmartelectronixDisplay.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/SmartelectronixDisplayEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/SmartelectronixDisplayEditor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/SmartelectronixDisplayMain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../Smexoscope/WaveDisplay.cpp
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

build_vst_gui(Smexoscope-dark "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
