#==============================================================================
# Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   January 2026
#
# VneIo - Mesh and Image library (from VertexNova core)
#==============================================================================

cmake_minimum_required(VERSION 3.16)
project(vneio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#-----------------------------------------------------------------------------
# Paths
#-----------------------------------------------------------------------------
set(VNEIO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(VNEIO_INCLUDE_DIR ${VNEIO_ROOT}/include)
set(VNEIO_SRC_DIR ${VNEIO_ROOT}/src)
set(VNEIO_EXTERNAL_DIR ${VNEIO_ROOT}/external)
set(VNEIO_THIRD_PARTY_DIR ${VNEIO_ROOT}/3rd_party)
set(VNEIO_TESTDATA_DIR "${VNEIO_ROOT}/testdata/vneresources/resources")

# Configure config.h for path_utils and tests
configure_file(${VNEIO_ROOT}/config/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

# VneCMake shared modules (submodule); then project-specific cmake
if(EXISTS "${VNEIO_ROOT}/cmake/vnecmake/modules")
    list(APPEND CMAKE_MODULE_PATH "${VNEIO_ROOT}/cmake/vnecmake/modules")
endif()
list(APPEND CMAKE_MODULE_PATH "${VNEIO_ROOT}/cmake")

#-----------------------------------------------------------------------------
# Build options
#-----------------------------------------------------------------------------
option(VNEIO_BUILD_TESTS "Build VneIo tests" OFF)
option(VNEIO_BUILD_EXAMPLES "Build VneIo examples" OFF)
option(VNEIO_USE_LOGGING "Use vnelogging when available" ON)
option(VNEIO_BUILD_MESH "Build mesh component" ON)
option(VNEIO_BUILD_IMAGE "Build image component" ON)

#-----------------------------------------------------------------------------
# Platform detection
#-----------------------------------------------------------------------------
if(NOT DEFINED VNE_TARGET_PLATFORM)
    if(EMSCRIPTEN)
        set(VNE_TARGET_PLATFORM "Web")
    elseif(WIN32)
        set(VNE_TARGET_PLATFORM "Windows")
    elseif(APPLE)
        if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
            set(VNE_TARGET_PLATFORM "iOS")
        else()
            set(VNE_TARGET_PLATFORM "macOS")
        endif()
    elseif(UNIX)
        if(CMAKE_SYSTEM_NAME STREQUAL "Android")
            set(VNE_TARGET_PLATFORM "Android")
        else()
            set(VNE_TARGET_PLATFORM "Linux")
        endif()
    else()
        set(VNE_TARGET_PLATFORM "Unknown")
    endif()
endif()
message(STATUS "VneIo: platform ${VNE_TARGET_PLATFORM}")

#-----------------------------------------------------------------------------
# Compiler warnings (interface library)
#-----------------------------------------------------------------------------
add_library(VneIoWarnings INTERFACE)
target_compile_options(VneIoWarnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
add_library(vne::io::Warnings ALIAS VneIoWarnings)

add_library(VneIoBuildSettings INTERFACE)
add_library(vne::io::BuildSettings ALIAS VneIoBuildSettings)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(VneIoBuildSettings INTERFACE VNE_BUILD_TYPE_DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(VneIoBuildSettings INTERFACE VNE_BUILD_TYPE_RELEASE)
endif()

#-----------------------------------------------------------------------------
# Optional: vnelogging (when VNEIO_USE_LOGGING and libs present)
#-----------------------------------------------------------------------------
set(VNE_LIBS_DIR ${VNEIO_ROOT}/libs)
if(VNEIO_USE_LOGGING AND EXISTS "${VNE_LIBS_DIR}/vnelogging/CMakeLists.txt")
    if(NOT TARGET vne::common AND EXISTS "${VNE_LIBS_DIR}/vnecommon/CMakeLists.txt")
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(libs/vnecommon libs/vnecommon)
    endif()
    if(NOT TARGET vne::logging)
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(libs/vnelogging ${CMAKE_BINARY_DIR}/libs/vnelogging)
        set(BUILD_TESTS ${VNEIO_BUILD_TESTS} CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES ${VNEIO_BUILD_EXAMPLES} CACHE BOOL "" FORCE)
    endif()
    set(VNEIO_HAS_LOGGING ON)
    message(STATUS "VneIo: using vnelogging from libs/vnelogging")
else()
    set(VNEIO_HAS_LOGGING OFF)
    add_compile_definitions(VNEIO_NO_LOGGING)
    message(STATUS "VneIo: building without vnelogging (no-op logging)")
endif()

#-----------------------------------------------------------------------------
# External / third-party: Assimp (for mesh)
# Prefer external/assimp, then 3rd_party/assimp, then find_package
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_MESH)
    if(EXISTS "${VNEIO_EXTERNAL_DIR}/assimp/CMakeLists.txt")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(external/assimp external/assimp EXCLUDE_FROM_ALL)
        set(VNEIO_ASSIMP_TARGET assimp)
        set(VNEIO_ASSIMP_INCLUDE "${VNEIO_EXTERNAL_DIR}/assimp/include")
        message(STATUS "VneIo: using Assimp from external/assimp")
    elseif(EXISTS "${VNEIO_THIRD_PARTY_DIR}/assimp/CMakeLists.txt")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(3rd_party/assimp 3rd_party/assimp EXCLUDE_FROM_ALL)
        set(VNEIO_ASSIMP_TARGET assimp)
        set(VNEIO_ASSIMP_INCLUDE "${VNEIO_THIRD_PARTY_DIR}/assimp/include")
        message(STATUS "VneIo: using Assimp from 3rd_party/assimp")
    else()
        find_package(assimp QUIET)
        if(assimp_FOUND)
            set(VNEIO_ASSIMP_TARGET assimp::assimp)
            set(VNEIO_ASSIMP_INCLUDE "")
            message(STATUS "VneIo: using Assimp from find_package")
        else()
            message(WARNING "VneIo: Assimp not found. Mesh loader will not be available. Add external/assimp, 3rd_party/assimp, or set assimp_DIR.")
        endif()
    endif()
endif()

#-----------------------------------------------------------------------------
# External / third-party: stb_image (for image)
# Prefer external/stb_image, then 3rd_party/stb_image, then FetchContent
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_IMAGE)
    set(VNEIO_STB_TARGET "")
    set(VNEIO_STB_INCLUDE "")
    if(EXISTS "${VNEIO_EXTERNAL_DIR}/stb_image/CMakeLists.txt")
        set(VNE_TARGET_PLATFORM ${VNE_TARGET_PLATFORM})
        add_subdirectory(external/stb_image external/stb_image)
        set(VNEIO_STB_TARGET stb_image)
        set(VNEIO_STB_INCLUDE "${VNEIO_EXTERNAL_DIR}/stb_image/src")
        message(STATUS "VneIo: using stb_image from external/stb_image")
        # Do not define VNEIO_STB_HEADER_ONLY: we use the library's implementation
    elseif(EXISTS "${VNEIO_THIRD_PARTY_DIR}/stb_image/CMakeLists.txt")
        set(VNE_TARGET_PLATFORM ${VNE_TARGET_PLATFORM})
        add_subdirectory(3rd_party/stb_image 3rd_party/stb_image)
        set(VNEIO_STB_TARGET stb_image)
        set(VNEIO_STB_INCLUDE "${VNEIO_THIRD_PARTY_DIR}/stb_image/src")
        message(STATUS "VneIo: using stb_image from 3rd_party/stb_image")
    else()
        # FetchContent from nothings/stb (header-only, no CMake)
        include(FetchContent)
        FetchContent_Declare(
            stb
            GIT_REPOSITORY https://github.com/nothings/stb.git
            GIT_TAG        master
        )
        FetchContent_MakeAvailable(stb)
        set(VNEIO_STB_INCLUDE ${stb_SOURCE_DIR})
        message(STATUS "VneIo: using stb from FetchContent")
        set(VNEIO_STB_HEADER_ONLY ON)
    endif()
endif()

#-----------------------------------------------------------------------------
# Mesh library
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_MESH AND VNEIO_ASSIMP_TARGET)
    add_library(vneio_mesh STATIC
        src/vertexnova/io/mesh/assimp_loader.cpp
    )
    target_include_directories(vneio_mesh
        PUBLIC
            $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${VNEIO_ASSIMP_INCLUDE}
    )
    target_link_libraries(vneio_mesh
        PUBLIC VneIoWarnings VneIoBuildSettings
        PRIVATE ${VNEIO_ASSIMP_TARGET}
    )
    if(VNEIO_HAS_LOGGING AND TARGET vne::logging)
        target_link_libraries(vneio_mesh PRIVATE vne::logging)
        target_compile_definitions(vneio_mesh PRIVATE VNEIO_HAS_LOGGING=1)
    endif()
    add_library(vne::io::mesh ALIAS vneio_mesh)
endif()

#-----------------------------------------------------------------------------
# Image library
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_IMAGE AND VNEIO_STB_INCLUDE)
    add_library(vneio_image STATIC
        src/vertexnova/io/image/image.cpp
    )
    target_include_directories(vneio_image
        PUBLIC
            $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${VNEIO_STB_INCLUDE}
    )
    if(VNEIO_STB_HEADER_ONLY)
        target_compile_definitions(vneio_image PRIVATE VNEIO_STB_HEADER_ONLY)
    endif()
    target_link_libraries(vneio_image
        PUBLIC VneIoWarnings VneIoBuildSettings
    )
    if(VNEIO_STB_TARGET)
        target_link_libraries(vneio_image PRIVATE ${VNEIO_STB_TARGET})
    endif()
    add_library(vne::io::image ALIAS vneio_image)
endif()

#-----------------------------------------------------------------------------
# Combined vneio target (for convenience)
#-----------------------------------------------------------------------------
add_library(vneio INTERFACE)
target_include_directories(vneio INTERFACE $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}> $<INSTALL_INTERFACE:include>)
if(TARGET vneio_mesh)
    target_link_libraries(vneio INTERFACE vneio_mesh)
endif()
if(TARGET vneio_image)
    target_link_libraries(vneio INTERFACE vneio_image)
endif()
add_library(vne::io ALIAS vneio)

#-----------------------------------------------------------------------------
# Tests & examples
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_TESTS AND EXISTS "${VNEIO_ROOT}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
if(VNEIO_BUILD_EXAMPLES AND EXISTS "${VNEIO_ROOT}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------
include(GNUInstallDirs)
set(VNEIO_INSTALL_TARGETS VneIoWarnings VneIoBuildSettings)
if(TARGET vneio_mesh)
    list(APPEND VNEIO_INSTALL_TARGETS vneio_mesh)
endif()
if(TARGET vneio_image)
    list(APPEND VNEIO_INSTALL_TARGETS vneio_image)
    # stb_image is a link dependency of vneio_image when using external/stb_image; include in export
    if(TARGET stb_image)
        list(APPEND VNEIO_INSTALL_TARGETS stb_image)
    endif()
endif()
install(TARGETS ${VNEIO_INSTALL_TARGETS} vneio
    EXPORT VneIoTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/vertexnova/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vertexnova FILES_MATCHING PATTERN "*.h")
install(EXPORT VneIoTargets FILE VneIoTargets.cmake NAMESPACE vne:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VneIo)
