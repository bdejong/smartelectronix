cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(H2O)

set(PROJECT_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Compressor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Compressor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oedit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oedit.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oeditor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oeditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oeffect.h
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Oeffect.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/H2Omain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.cpp
)

set(PROJECT_IMAGES
  ${CMAKE_CURRENT_SOURCE_DIR}/images/ascii.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/back.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/knob.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/saturate.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/splash.png
)

build_vst_gui(H2O "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
