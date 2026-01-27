cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(CrazyIvan)

set(PROJECT_SOURCE
  EnvFollower.cpp
  EnvFollower.h
  Flanger.cpp
  Flanger.h
  MovingDelay.h
  OnePoleLP.cpp
  OnePoleLP.h
  TriOsc.cpp
  TriOsc.h
  vstxsynth.cpp
  vstxsynth.h
  vstxsynthmain.cpp
)

build_vst_nogui(CrazyIvan "${PROJECT_SOURCE}")
