cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

project(OnePingOnly)

set(PROJECT_SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/defs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Delay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Delay.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PingSynth.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PingSynth.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PingSynthProc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/APingSynthMain.cpp
)

build_vst_nogui(OnePingOnly "${PROJECT_SOURCE}")
