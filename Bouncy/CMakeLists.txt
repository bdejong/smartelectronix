cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

project(Bouncy)

set(PROJECT_SOURCE
  Bouncy.cpp
  Multitap.cpp
  Registry.cpp
  vstxsynth.cpp
  vstxsynthmain.cpp
  vstxsynthproc.cpp
)

build_vst_nogui(Bouncy "${PROJECT_SOURCE}")
