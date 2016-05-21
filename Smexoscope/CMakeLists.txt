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

# VST GUI

set(VSTGUI_DIR ../vstgui/vstgui)

include_directories(${VSTGUI_DIR})
include_directories(${VSTGUI_DIR}/plugin-bindings)

set(VST_GUI_SOURCE_CODE
  ${VSTGUI_DIR}/vstgui.cpp
  ${VSTGUI_DIR}/lib/cbitmap.cpp
  ${VSTGUI_DIR}/lib/cbitmapfilter.cpp
  ${VSTGUI_DIR}/lib/ccolor.cpp
  ${VSTGUI_DIR}/lib/cdatabrowser.cpp
  ${VSTGUI_DIR}/lib/cdrawcontext.cpp
  ${VSTGUI_DIR}/lib/cdrawmethods.cpp
  ${VSTGUI_DIR}/lib/cdropsource.cpp
  ${VSTGUI_DIR}/lib/cfileselector.cpp
  ${VSTGUI_DIR}/lib/cfont.cpp
  ${VSTGUI_DIR}/lib/cframe.cpp
  ${VSTGUI_DIR}/lib/cgradientview.cpp
  ${VSTGUI_DIR}/lib/cgraphicspath.cpp
  ${VSTGUI_DIR}/lib/clayeredviewcontainer.cpp
  ${VSTGUI_DIR}/lib/clinestyle.cpp
  ${VSTGUI_DIR}/lib/coffscreencontext.cpp
  ${VSTGUI_DIR}/lib/copenglview.cpp
  ${VSTGUI_DIR}/lib/cpoint.cpp
  ${VSTGUI_DIR}/lib/crect.cpp
  ${VSTGUI_DIR}/lib/crowcolumnview.cpp
  ${VSTGUI_DIR}/lib/cscrollview.cpp
  ${VSTGUI_DIR}/lib/cshadowviewcontainer.cpp
  ${VSTGUI_DIR}/lib/csplitview.cpp
  ${VSTGUI_DIR}/lib/cstring.cpp
  ${VSTGUI_DIR}/lib/ctabview.cpp
  ${VSTGUI_DIR}/lib/ctooltipsupport.cpp
  ${VSTGUI_DIR}/lib/cview.cpp
  ${VSTGUI_DIR}/lib/cviewcontainer.cpp
  ${VSTGUI_DIR}/lib/cvstguitimer.cpp
  ${VSTGUI_DIR}/lib/vstguidebug.cpp
)

# Project settings

project(Smexoscope)

set(PROJECT_SOURCE_CODE
  Label.cpp
  MultiStateButton.cpp
  resource.rc
  SmartelectronixDisplay.cpp
  SmartelectronixDisplayEditor.cpp
  WaveDisplay.cpp
)

add_library(Smexoscope MODULE ${PROJECT_SOURCE_CODE} ${VST_SOURCE_CODE} ${PKG_INFO} ${VST_GUI_SOURCE_CODE})

set_target_properties(Smexoscope PROPERTIES
  BUNDLE true
  BUNDLE_EXTENSION vst
  MACOSX_BUNDLE_INFO_PLIST ${COMMON_DIR}/Info.plist.in
)

target_compile_features(Smexoscope PRIVATE cxx_range_for)
