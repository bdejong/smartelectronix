# Global settings
cmake_minimum_required(VERSION 3.5)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Common)
set(STEINBERG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Steinberg)
set(VST_CPP_PATH ${STEINBERG_DIR}/public.sdk/source/vst2.x)

set(VST_SOURCE_CODE
  ${VST_CPP_PATH}/audioeffect.cpp
  ${VST_CPP_PATH}/audioeffectx.cpp
  ${VST_CPP_PATH}/vstplugmain.cpp
)

set(PKG_INFO ${COMMON_DIR}/PkgInfo)

set_source_files_properties(${COMMON_DIR}/PkgInfo PROPERTIES
  MACOSX_PACKAGE_LOCATION .
)

set(CMAKE_OSX_ARCHITECTURES "i386" "x86_64")

# VST GUI
set(VSTGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vstgui/vstgui)

include_directories(${VSTGUI_DIR})
include_directories(${VSTGUI_DIR}/plugin-bindings)
include_directories(${STEINBERG_DIR})
include_directories(${VST_CPP_PATH})

set(VST_GUI_SOURCE_CODE
  ${VSTGUI_DIR}/vstgui_mac.mm
  ${VSTGUI_DIR}/plugin-bindings/aeffguieditor.cpp
)

IF(APPLE)
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  find_library(ACCELERATE Accelerate)
  find_library(QUARTZ QuartzCore)
  set(SYSTEM_LIBS ${CARBON} ${COCOA} ${OPENGL} ${ACCELERATE} ${QUARTZ})
ELSE (APPLE)
  set(SYSTEM_LIBS)
ENDIF (APPLE)

# Project settings

project(Smexoscope)

set(PROJECT_SOURCE_CODE
  SmartelectronixDisplayMain.cpp
  SmartelectronixDisplay.cpp
  SmartelectronixDisplayEditor.cpp
  Label.cpp
  MultiStateButton.cpp
  WaveDisplay.cpp
  resource.rc
)

add_library(Smexoscope MODULE ${PROJECT_SOURCE_CODE} ${VST_SOURCE_CODE} ${PKG_INFO} ${VST_GUI_SOURCE_CODE})

target_link_libraries(Smexoscope ${SYSTEM_LIBS})

set_target_properties(Smexoscope PROPERTIES
  BUNDLE true
  BUNDLE_EXTENSION vst
  MACOSX_BUNDLE_INFO_PLIST ${COMMON_DIR}/Info.plist.in
)

set_property(TARGET Smexoscope PROPERTY CXX_STANDARD 11)
