cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/common.cmake)

pre_build()

project(BitMurderer)

set(PROJECT_SOURCE
${CMAKE_CURRENT_SOURCE_DIR}/MurderEditor.cpp
${CMAKE_CURRENT_SOURCE_DIR}/MurderEditor.h
${CMAKE_CURRENT_SOURCE_DIR}/bitmurderer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/bitmurderer.h
${CMAKE_CURRENT_SOURCE_DIR}/bitmurderermain.cpp
)

set(PROJECT_IMAGES
  ${CMAKE_CURRENT_SOURCE_DIR}/images/1.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/2.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/bitmurderer.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/credits.png
)

build_vst_gui(BitMurderer "${PROJECT_SOURCE}" "${PROJECT_IMAGES}")
