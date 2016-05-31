# Global settings --------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Common)
set(STEINBERG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Steinberg)
set(VSTGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vstgui/vstgui)
include_directories(${STEINBERG_DIR})
include_directories(${VSTGUI_DIR})

# VST Interface ----------------------------------------------------------------
set(VST_SOURCE
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/audioeffect.cpp
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/audioeffectx.cpp
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/vstplugmain.cpp
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/aeffeditor.h
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/audioeffect.h
  ${STEINBERG_DIR}/public.sdk/source/vst2.x/audioeffectx.h
)

# VST GUI ----------------------------------------------------------------------
set(VSTGUI_SOURCE 
  ${VSTGUI_DIR}/plugin-bindings/aeffguieditor.cpp
  ${VSTGUI_DIR}/plugin-bindings/aeffguieditor.h
)

IF(WIN32)
  list(APPEND VSTGUI_SOURCE ${VSTGUI_DIR}/vstgui_win32.cpp)
ELSEIF(APPLE)
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  find_library(ACCELERATE Accelerate)
  find_library(QUARTZ QuartzCore)
  set(SYSTEM_LIBS ${CARBON} ${COCOA} ${OPENGL} ${ACCELERATE} ${QUARTZ})
  list(APPEND VSTGUI_SOURCE ${VSTGUI_DIR}/vstgui_mac.mm)
ENDIF(WIN32)

# VST Interface ----------------------------------------------------------------
set(VST_INTERFACE
  ${STEINBERG_DIR}/pluginterfaces/vst2.x/aeffect.h
  ${STEINBERG_DIR}/pluginterfaces/vst2.x/aeffectx.h
  ${STEINBERG_DIR}/pluginterfaces/vst2.x/vstfxstore.h
)

# Project Source ---------------------------------------------------------------
set(PROJECT_SOURCE
  SmartelectronixDisplayMain.cpp
  SmartelectronixDisplay.cpp
  SmartelectronixDisplayEditor.cpp
  Label.cpp
  MultiStateButton.cpp
  WaveDisplay.cpp
  SmartelectronixDisplay.hpp
  SmartelectronixDisplayEditor.h
  Label.h
  MultiStateButton.h
  WaveDisplay.h
  Defines.h
)

# Project Settings -------------------------------------------------------------
IF(WIN32)
  project(Smexoscope)
  add_library(Smexoscope MODULE 
    ${PROJECT_SOURCE} 
    ${VST_SOURCE} 
    ${VSTGUI_SOURCE} 
    ${VST_INTERFACE}
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/OscilloscopeModular/OscilloscopeModular.def
  )
  
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE=1)
  
  source_group("vstgui" FILES ${VSTGUI_SOURCE})
  source_group("vst2.x" FILES ${VST_SOURCE})
  source_group("Interfaces" FILES ${VST_INTERFACE})
  
ELSEIF(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "i386" "x86_64")

  set(PKG_INFO ${COMMON_DIR}/PkgInfo)
  set_source_files_properties(${COMMON_DIR}/PkgInfo PROPERTIES
    MACOSX_PACKAGE_LOCATION .
  )
  
  set(PROJECT_IMAGES
    images/blue_knob1_4.png
    images/body.png
    images/free_etc.png
    images/heads.png
    images/lefr_right.png
    images/off_on.png
    images/readout.png
    images/readout2.png
    images/slider_new.png
  )
  
  project(Smexoscope)
  set_source_files_properties(${PROJECT_IMAGES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  add_library(Smexoscope MODULE ${PROJECT_SOURCE} ${PROJECT_IMAGES} ${VST_SOURCE} ${PKG_INFO} ${VSTGUI_SOURCE})
  target_link_libraries(Smexoscope ${SYSTEM_LIBS})

  set_target_properties(Smexoscope PROPERTIES
    BUNDLE true
    BUNDLE_EXTENSION vst
    MACOSX_BUNDLE_INFO_PLIST ${COMMON_DIR}/Info.plist.in
  )
  set_property(TARGET Smexoscope PROPERTY CXX_STANDARD 11)
ENDIF(WIN32)
