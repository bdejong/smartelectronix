cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

project(SupaTrigger)

set(PROJECT_SOURCE
  SupaTriggerMain.cpp
  SupaTrigger.cpp
  SupaTrigger.hpp
)

build_vst_nogui(SupaTrigger "${PROJECT_SOURCE}")
