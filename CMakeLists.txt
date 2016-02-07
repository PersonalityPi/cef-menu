cmake_minimum_required(VERSION 2.8.12.1)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
project(cef)
set_property(GLOBAL PROPERTY OS_FOLDERS ON)
set(OS_WINDOWS 1)
set(PROJECT_ARCH "x86")

if(NOT CMAKE_BUILD_TYPE AND
   (${CMAKE_GENERATOR} STREQUAL "Ninja" OR ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles"))
  # CMAKE_BUILD_TYPE should be specified when using Ninja or Unix Makefiles.
  set(CMAKE_BUILD_TYPE Release)
  message(WARNING "No CMAKE_BUILD_TYPE value selected, using ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
include("macros")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

if(OS_WINDOWS)
  set(CEF_DEBUG_INFO_FLAG "/Zi" CACHE STRING "Optional flag specifying specific /Z flag to use")
  set(CEF_LIBTYPE                 STATIC)
  set(CEF_COMPILER_FLAGS          "/MP /Gy /GR- /W4 /WX /wd\"4100\" /wd\"4127\" /wd\"4244\" /wd\"4512\" /wd\"4701\" /wd\"4702\" /wd\"4996\" ${CEF_DEBUG_INFO_FLAG}")
  set(CEF_COMPILER_FLAGS_DEBUG    "/MTd /RTC1 /Od")
  set(CEF_COMPILER_FLAGS_RELEASE  "/MT /O2 /Ob2 /GF /D NDEBUG /D _NDEBUG")
  set(CEF_LINKER_FLAGS_DEBUG      "/DEBUG")
  set(CEF_EXE_LINKER_FLAGS        "/MANIFEST:NO")
  add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -DUNICODE -D_UNICODE -DWINVER=0x0602
                  -D_WIN32_WINNT=0x602 -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_HAS_EXCEPTIONS=0)

  set(CEF_STANDARD_LIBS "comctl32.lib" "rpcrt4.lib" "shlwapi.lib")
  set(CEF_RESOURCE_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
  set(CEF_BINARY_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIGURATION>")
  set(CEF_BINARY_DIR_DEBUG    "${CMAKE_CURRENT_SOURCE_DIR}/Debug")
  set(CEF_BINARY_DIR_RELEASE  "${CMAKE_CURRENT_SOURCE_DIR}/Release")
  set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.lib")
  set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.lib")

  set(CEF_BINARY_FILES
    d3dcompiler_43.dll
    d3dcompiler_47.dll
    libcef.dll
    libEGL.dll
    libGLESv2.dll
    natives_blob.bin
    snapshot_blob.bin
    wow_helper.exe
    )

  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
    )

  option(USE_SANDBOX "Enable or disable use of the sandbox." ON)
  if(USE_SANDBOX AND NOT MSVC_VERSION EQUAL 1800)
    set(USE_SANDBOX OFF)
  endif()

  if(USE_SANDBOX)
    add_definitions(-DPSAPI_VERSION=1)
    add_definitions(-DCEF_USE_SANDBOX)
    set(CEF_SANDBOX_STANDARD_LIBS "dbghelp.lib" "psapi.lib")
    set(CEF_SANDBOX_LIB_DEBUG "${CEF_BINARY_DIR_DEBUG}/cef_sandbox.lib")
    set(CEF_SANDBOX_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/cef_sandbox.lib")
  endif()

  option(USE_ATL "Enable or disable use of ATL." ON)
  if(USE_ATL)
    get_filename_component(VC_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(VC_DIR ${VC_BIN_DIR} DIRECTORY)
    if(NOT IS_DIRECTORY "${VC_DIR}/atlmfc")
      set(USE_ATL OFF)
    endif()
  endif()

  if(USE_ATL)
    add_definitions(-DCEF_USE_ATL)
  endif()
endif()

set(CMAKE_C_FLAGS                     "${CEF_COMPILER_FLAGS} ${CEF_C_COMPILER_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG               "${CEF_COMPILER_FLAGS_DEBUG} ${CEF_C_COMPILER_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE             "${CEF_COMPILER_FLAGS_RELEASE} ${CEF_C_COMPILER_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS                   "${CEF_COMPILER_FLAGS} ${CEF_CXX_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG             "${CEF_COMPILER_FLAGS_DEBUG} ${CEF_CXX_COMPILER_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE           "${CEF_COMPILER_FLAGS_RELEASE} ${CEF_CXX_COMPILER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS            "${CEF_LINKER_FLAGS} ${CEF_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG      "${CEF_LINKER_FLAGS_DEBUG} ${CEF_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CEF_LINKER_FLAGS_RELEASE} ${CEF_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS         "${CEF_LINKER_FLAGS} ${CEF_SHARED_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG   "${CEF_LINKER_FLAGS_DEBUG} ${CEF_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CEF_LINKER_FLAGS_RELEASE} ${CEF_SHARED_LINKER_FLAGS_RELEASE}")

add_subdirectory(libcef_dll)
add_subdirectory(cefclient)
add_subdirectory(cefsimple)

message(STATUS "*** CONFIGURATION SETTINGS ***")
message(STATUS "Generator:                    ${CMAKE_GENERATOR}")
message(STATUS "Platform:                     ${CMAKE_SYSTEM_NAME}")
message(STATUS "Project architecture:         ${PROJECT_ARCH}")

if(${CMAKE_GENERATOR} STREQUAL "Ninja" OR ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  message(STATUS "Build type:                   ${CMAKE_BUILD_TYPE}")
endif()

if(OS_MACOSX)
  message(STATUS "Base SDK:                     ${CMAKE_OSX_SYSROOT}")
  message(STATUS "Target SDK:                   ${CEF_TARGET_SDK}")
endif()

if(OS_WINDOWS)
  message(STATUS "CEF Windows sandbox:          ${USE_SANDBOX}")
  message(STATUS "Visual Studio ATL support:    ${USE_ATL}")
endif()

set(LIBRARIES ${CEF_STANDARD_LIBS})
if(OS_WINDOWS AND USE_SANDBOX)
  set(LIBRARIES ${LIBRARIES} ${CEF_SANDBOX_STANDARD_LIBS})
endif()
message(STATUS "Standard libraries:           ${LIBRARIES}")

get_directory_property(DEFINITIONS COMPILE_DEFINITIONS)
message(STATUS "Compiler definitions:         ${DEFINITIONS}")

message(STATUS "C_FLAGS:                      ${CMAKE_C_FLAGS}")
message(STATUS "C_FLAGS_DEBUG:                ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "C_FLAGS_RELEASE:              ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CXX_FLAGS:                    ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG:              ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE:            ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "EXE_LINKER_FLAGS:             ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "EXE_LINKER_FLAGS_DEBUG:       ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
message(STATUS "EXE_LINKER_FLAGS_RELEASE:     ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
message(STATUS "SHARED_LINKER_FLAGS:          ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "SHARED_LINKER_FLAGS_DEBUG:    ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
message(STATUS "SHARED_LINKER_FLAGS_RELEASE:  ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")

if(OS_LINUX OR OS_WINDOWS)
  message(STATUS "CEF Binary files:             ${CEF_BINARY_FILES}")
  message(STATUS "CEF Resource files:           ${CEF_RESOURCE_FILES}")
endif()
