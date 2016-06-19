cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

project(SupaPhaser)

set(PROJECT_SOURCE 
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaEditor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaPhaser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaPhaser.def
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaPhaser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaPhaserMain.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CustomSplashScreen.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/CustomSplashScreen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MultiStateButton.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/MultiStateButton.h
  ${CMAKE_CURRENT_SOURCE_DIR}/presets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextDisplay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WavetableFPOsc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/WavetableFPOsc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/asciitable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ASupaEditor.cpp
)

set(PROJECT_IMAGES
  ${CMAKE_CURRENT_SOURCE_DIR}/images/blue_knob1_4.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/extend_on_off.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/heads.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/slider.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/small_knob02.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/splash.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/type_grey-back_white.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/type_orange-back.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/type_white-back.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/base.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/bigknob01rotated_cropped2.png
)

build_vst_gui(SupaPhaser "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
