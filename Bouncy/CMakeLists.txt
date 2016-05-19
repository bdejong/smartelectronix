# Global settings
cmake_minimum_required(VERSION 3.5)

set(COMMON_DIR ../Common)
set(STEINBERG_DIR ../Steinberg)
set(VST_CPP_PATH ${STEINBERG_DIR}/public.sdk/source/vst2.x)

include_directories(${STEINBERG_DIR})
include_directories(${VST_CPP_PATH})

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

# Project settings

project(Bouncy)

set(PROJECT_SOURCE_CODE
  Bouncy.cpp
  Multitap.cpp
  Registry.cpp
  vstxsynth.cpp
  vstxsynthmain.cpp
  vstxsynthproc.cpp
)

add_library(Bouncy MODULE ${PROJECT_SOURCE_CODE} ${VST_SOURCE_CODE} ${PKG_INFO})

set_target_properties(Bouncy PROPERTIES
  BUNDLE true
  BUNDLE_EXTENSION vst
  MACOSX_BUNDLE_INFO_PLIST ${COMMON_DIR}/Info.plist.in
)
