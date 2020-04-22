project("Units FM Synth" CXX)

cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_definitions(/W4 /w14545 /w34242 /w34254 /w34287 /w44263 /w44265 /w44296 /w44311 /w44826 /we4289 /w14546 /w14547 /w14549 /w14555 /w14619 /w14905 /w14906 /w14928)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # add_definitions(-Wall -Wextra -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wcast-qual -Wunused -Woverloaded-virtual -Wno-noexcept-type -Wpedantic)
    add_definitions( -Wno-c++98-compat-pedantic 
                     -Wno-self-assign-overloaded 
                     -Wno-c++98-compat 
                     -Wno-documentation 
                     -Wno-switch-enum 
                     -Wno-weak-vtables 
                     -Wno-missing-prototypes 
                     -Wno-padded 
                     -Wno-missing-noreturn 
                     -Wno-c++17-extensions 
                     -Wno-c++14-extensions 
                     -Wno-c++11-extensions 
                     -Wno-exit-time-destructors 
                     -Wno-documentation-unknown-command 
                     -Wno-unused-template 
                     -Wno-undef )
endif()

#Add the files we need to build the parts of JUCE we depend on, as well as the libraries juce relies on
if(APPLE)
  # add_definitions(-Wno-sign-compare)

  add_library(JUCE_Dependencies SHARED
  ${JUCE_PATH}/juce_core/juce_core.mm 
  ${JUCE_PATH}/juce_data_structures/juce_data_structures.mm 
  ${JUCE_PATH}/juce_audio_basics/juce_audio_basics.mm
  ${JUCE_PATH}/juce_audio_devices/juce_audio_devices.mm
  ${JUCE_PATH}/juce_audio_formats/juce_audio_formats.mm
  ${JUCE_PATH}/juce_audio_processors/juce_audio_processors.mm
  ${JUCE_PATH}/juce_audio_utils/juce_audio_utils.mm
  ${JUCE_PATH}/juce_events/juce_events.mm 
  ${JUCE_PATH}/juce_dsp/juce_dsp.mm 
  ${JUCE_PATH}/juce_gui_basics/juce_gui_basics.mm 
  ${JUCE_PATH}/juce_graphics/juce_graphics.mm 
  ${JUCE_PATH}/juce_audio_basics/juce_audio_basics.mm 
  ${JUCE_PATH}/juce_audio_processors/juce_audio_processors.mm
  )

  target_link_libraries (JUCE_Dependencies PRIVATE
  "-framework Accelerate"
  "-framework AudioToolbox"
  "-framework Cocoa"
  "-framework CoreAudio"
  "-framework CoreAudioKit"
  "-framework CoreMIDI"
  "-framework IOKit"
  "-framework QuartzCore"
  )
else()
  add_library(JUCE_Dependencies STATIC
              ${JUCE_PATH}/juce_core/juce_core.cpp 
              ${JUCE_PATH}/juce_data_structures/juce_data_structures.cpp 
              ${JUCE_PATH}/juce_audio_basics/juce_audio_basics.cpp
              ${JUCE_PATH}/juce_audio_devices/juce_audio_devices.cpp
              ${JUCE_PATH}/juce_audio_formats/juce_audio_formats.cpp
              ${JUCE_PATH}/juce_audio_processors/juce_audio_processors.cpp
              ${JUCE_PATH}/juce_audio_utils/juce_audio_utils.cpp
              ${JUCE_PATH}/juce_events/juce_events.cpp 
              ${JUCE_PATH}/juce_dsp/juce_dsp.cpp 
              ${JUCE_PATH}/juce_gui_basics/juce_gui_basics.cpp 
              ${JUCE_PATH}/juce_graphics/juce_graphics.cpp 
              ${JUCE_PATH}/juce_audio_basics/juce_audio_basics.cpp 
              ${JUCE_PATH}/juce_audio_processors/juce_audio_processors.cpp)
  if(WIN32)              
    target_link_libraries(JUCE_Dependencies ws2_32.dll Imm32.dll Winmm.dll Shlwapi.dll Wininet.dll Version.dll)
  elseif(UNIX AND NOT APPLE)#This is how JUCE selects for linux
  target_compile_options(JUCE_Dependencies PRIVATE
                      -fno-common 
                      -Wswitch 
                      -Wunused-variable 
                      -Wparentheses 
                      -Wnon-virtual-dtor 
                      -Wshorten-64-to-32 
                      -Wundeclared-selector 
                      -Wuninitialized 
                      -Wunused-function 
                      -Wbool-conversion 
                      -Wcomma 
                      -Wconstant-conversion 
                      -Wempty-body 
                      -Wenum-conversion 
                      -Winfinite-recursion 
                      -Wint-conversion 
                      -Wrange-loop-analysis 
                      -Wstrict-prototypes 
                      -Wmove 
                      -Wunreachable-code 
                      -Wduplicate-method-match 
                      -Wreorder 
                      -fvisibility-inlines-hidden)
    find_package (PkgConfig REQUIRED)
      pkg_search_module (ALSA REQUIRED alsa)
      pkg_search_module (X11 REQUIRED x11)
      pkg_search_module (XINERAMA REQUIRED xinerama)
      pkg_search_module (XEXT REQUIRED xext)
      pkg_search_module (FREETYPE2 REQUIRED freetype2)
      pkg_search_module (WEBKIT2GTK-4.0 REQUIRED webkit2gtk-4.0)
      pkg_search_module (GTK+-X11-3.0 REQUIRED gtk+-x11-3.0)
      pkg_search_module (LIBCURL REQUIRED libcurl)
      include_directories(
        ${ALSA_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${XINERAMA_INCLUDE_DIRS}
        ${XEXT_INCLUDE_DIRS}
        ${FREETYPE2_INCLUDE_DIRS}
        ${WEBKIT2GTK-4.0_INCLUDE_DIRS}
        ${GTK+-X11-3.0_INCLUDE_DIRS}
        ${LIBCURL_INCLUDE_DIRS}
        )
      find_library (RT rt)
      find_library (DL dl)
      find_library (PTHREAD pthread)
      find_library (GL GL)
      target_link_libraries(JUCE_Dependencies   
        ${RT}
        ${DL}
        ${PTHREAD}
        ${GL}
        ${ALSA_LIBRARIES}
        ${X11_LIBRARIES}
        ${XINERAMA_LIBRARIES}
        ${XEXT_LIBRARIES}
        ${FREETYPE2_LIBRARIES}
        ${WEBKIT2GTK-4.0_LIBRARIES}
        ${GTK+-X11-3.0_LIBRARIES}
        ${LIBCURL_LIBRARIES}
        m
        stdc++)
  endif()
endif()

set_property(TARGET JUCE_Dependencies PROPERTY CXX_STANDARD 17)
#Compile the JUCE Dependencies with the same compile flags and macros JUCE uses
target_compile_options(JUCE_Dependencies PRIVATE
                      -DJUCE_MODULE_AVAILABLE_juce_core=1
                      -DJUCE_MODULE_AVAILABLE_juce_data_structures=1
                      -DJUCE_MODULE_AVAILABLE_juce_gui_basics=1
                      -DJUCE_MODULE_AVAILABLE_juce_events=1
                      -DJUCE_MODULE_AVAILABLE_juce_graphics=1
                      -DJUCE_MODULE_AVAILABLE_juce_dsp=1
                      -DJUCE_MODULE_AVAILABLE_juce_audio_basics=1
                      -DJUCE_MODULE_AVAILABLE_juce_audio_devices=1
                      -DJUCE_MODULE_AVAILABLE_juce_audio_formats=1
                      -DJUCE_MODULE_AVAILABLE_juce_audio_processors=1)
#JUCE's source files check for these options when compiled from another target, so they need to be public...
target_compile_options(JUCE_Dependencies PUBLIC
                      -DDEBUG=1
                      -D_DEBUG=1
                      -DNDEBUG=0
                      -DJUCE_STANDALONE_APPLICATION=1
                      -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0)

#Add the path to juce's modules to compiler's search paths
target_include_directories(JUCE_Dependencies PUBLIC "${JUCE_PATH}")

#make sure catch2 is installed
find_package(Catch2 REQUIRED)
#include Catch, so the test files find it
include(Catch)

#add cpp files to relevant targets
add_executable(DSPTests ../TestMain.cpp ../DSPTests.cpp)
add_executable(ParamTests ../TestMain.cpp ../ParameterTests.cpp) 
add_executable(DSPUtilitiesTests ../TestMain.cpp ../DSPUnitUtilities.cpp) 
add_executable(MIDIMPESeparatorTests ../TestMain.cpp ../MIDIMPESeparatorTests.cpp) 
add_executable(AllTests ../TestMain.cpp ../DSPTests.cpp ../ParameterTests.cpp ../DSPUnitUtilities.cpp ../MIDIMPESeparatorTests.cpp)

#link tests with JUCE and Catch
target_link_libraries(DSPTests PRIVATE JUCE_Dependencies Catch2::Catch2)
target_link_libraries(ParamTests JUCE_Dependencies Catch2::Catch2)
target_link_libraries(DSPUtilitiesTests JUCE_Dependencies Catch2::Catch2)
target_link_libraries(MIDIMPESeparatorTests JUCE_Dependencies Catch2::Catch2)
target_link_libraries(AllTests JUCE_Dependencies Catch2::Catch2)
