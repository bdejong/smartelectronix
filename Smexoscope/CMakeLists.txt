# Global settings --------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Common)
set(STEINBERG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Steinberg)
include_directories(${STEINBERG_DIR})

# VST Interface ----------------------------------------------------------------
set(VST_SOURCE_PATH ${STEINBERG_DIR}/public.sdk/source/vst2.x)
include_directories(${VST_SOURCE_PATH})
set(VST_SOURCE
  ${VST_SOURCE_PATH}/audioeffect.cpp
  ${VST_SOURCE_PATH}/audioeffectx.cpp
  ${VST_SOURCE_PATH}/vstplugmain.cpp
  ${VST_SOURCE_PATH}/aeffeditor.h
  ${VST_SOURCE_PATH}/audioeffect.h
  ${VST_SOURCE_PATH}/audioeffectx.h
)

# VST GUI ----------------------------------------------------------------------
set(VSTGUI_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../vstgui/vstgui)
include_directories(${VSTGUI_SOURCE_PATH})
include_directories(${VSTGUI_SOURCE_PATH}/plugin-bindings)
set(VSTGUI_SOURCE 
  ${VSTGUI_SOURCE_PATH}/plugin-bindings/aeffguieditor.cpp
  ${VSTGUI_SOURCE_PATH}/plugin-bindings/aeffguieditor.h
)

IF(WIN32)
  list(APPEND VSTGUI_SOURCE ${VSTGUI_SOURCE_PATH}/vstgui_win32.cpp)
ELSEIF(APPLE)
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  find_library(ACCELERATE Accelerate)
  find_library(QUARTZ QuartzCore)
  set(SYSTEM_LIBS ${CARBON} ${COCOA} ${OPENGL} ${ACCELERATE} ${QUARTZ})
  list(APPEND VSTGUI_SOURCE ${VSTGUI_SOURCE_PATH}/vstgui_mac.mm)
ENDIF(WIN32)

# VST Interface ----------------------------------------------------------------
set(VST_INTERFACE_PATH ${STEINBERG_DIR}/pluginterfaces/vst2.x)
set(VST_INTERFACE
  ${VST_INTERFACE_PATH}/aeffect.h
  ${VST_INTERFACE_PATH}/aeffectx.h
  ${VST_INTERFACE_PATH}/vstfxstore.h
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
)

# Project Settings -------------------------------------------------------------
IF(WIN32)
  set(RESOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc
  )
  
  #TODO change based on SIMPLE_VERSION definition
  set(DEF_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/OscilloscopeModular/OscilloscopeModular.def
  )
  
  project(Smexoscope)
  add_library(Smexoscope MODULE 
    ${PROJECT_SOURCE} 
    ${VST_SOURCE} 
    ${VSTGUI_SOURCE} 
    ${RESOURCE_FILES}
    ${VST_INTERFACE}
    ${DEF_FILE}
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
  
  set(PROJECT_BITMAPS
    bitmaps/blue_knob1_4.bmp
    bitmaps/body.bmp
    bitmaps/free_etc.bmp
    bitmaps/heads.bmp
    bitmaps/lefr_right.bmp
    bitmaps/off_on.bmp
    bitmaps/readout.bmp
    bitmaps/readout2.bmp
    bitmaps/slider_new.bmp
  )
  
  project(Smexoscope)
  set_source_files_properties(${PROJECT_BITMAPS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  add_library(Smexoscope MODULE ${PROJECT_SOURCE} ${PROJECT_BITMAPS} ${VST_SOURCE} ${PKG_INFO} ${VSTGUI_SOURCE})
  target_link_libraries(Smexoscope ${SYSTEM_LIBS})

  set_target_properties(Smexoscope PROPERTIES
    BUNDLE true
    BUNDLE_EXTENSION vst
    MACOSX_BUNDLE_INFO_PLIST ${COMMON_DIR}/Info.plist.in
  )
  set_property(TARGET Smexoscope PROPERTY CXX_STANDARD 11)
ENDIF(WIN32)
