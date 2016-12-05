cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(SupaTrigga)

set(PROJECT_SOURCE
  SupaTriggerMain.cpp
  SupaTrigger.cpp
  SupaTrigger.hpp
)

build_vst_nogui(SupaTrigga "${PROJECT_SOURCE}")
