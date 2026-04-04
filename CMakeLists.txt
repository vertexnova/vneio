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
set(VNEIO_EXTERNAL_DIR ${VNEIO_ROOT}/deps/external)
set(VNEIO_THIRD_PARTY_DIR ${VNEIO_ROOT}/3rd_party)
set(VNEIO_TESTDATA_DIR "${VNEIO_ROOT}/testdata")

# VneIo-generated headers (vneio_config.h — vnelogging overwrites config.h in the same dir)
set(VNEIO_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test_output")
configure_file(${VNEIO_ROOT}/config/config.h.in ${CMAKE_BINARY_DIR}/vneio_config.h @ONLY)

# VneCMake shared modules (submodule); then project-specific cmake
if(EXISTS "${VNEIO_ROOT}/cmake/vnecmake/modules")
    list(APPEND CMAKE_MODULE_PATH "${VNEIO_ROOT}/cmake/vnecmake/modules")
endif()
list(APPEND CMAKE_MODULE_PATH "${VNEIO_ROOT}/cmake")

#-----------------------------------------------------------------------------
# Library type (one per build): static or shared for all VneIo component libs
#-----------------------------------------------------------------------------
set(VNEIO_LIB_TYPE "static" CACHE STRING "Library type for VneIo component libs: static or shared (one per build)")
set_property(CACHE VNEIO_LIB_TYPE PROPERTY STRINGS static shared)
if(NOT VNEIO_LIB_TYPE STREQUAL "static" AND NOT VNEIO_LIB_TYPE STREQUAL "shared")
    message(FATAL_ERROR "VNEIO_LIB_TYPE must be 'static' or 'shared'. Got: ${VNEIO_LIB_TYPE}")
endif()
if(VNEIO_LIB_TYPE STREQUAL "shared")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
if(VNEIO_LIB_TYPE STREQUAL "shared")
    set(_VNEIO_LIB_KIND SHARED)
else()
    set(_VNEIO_LIB_KIND STATIC)
endif()

# CI and recommended local layout: cmake -B build/<static|shared>/<Release|Debug> (all platforms).
message(STATUS "VneIo: VNEIO_LIB_TYPE=${VNEIO_LIB_TYPE} — typical -B build/${VNEIO_LIB_TYPE}/Release or .../Debug")

function(vneio_target_apply_shared_export target_name)
    if(_VNEIO_LIB_KIND STREQUAL "SHARED")
        target_compile_definitions(${target_name}
            PRIVATE VNEIO_BUILDING_DLL
            INTERFACE VNEIO_DLL
        )
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /wd4251 /wd4275)
        else()
            set_target_properties(${target_name} PROPERTIES
                CXX_VISIBILITY_PRESET hidden
                VISIBILITY_INLINES_HIDDEN ON
            )
        endif()
    endif()
endfunction()

#-----------------------------------------------------------------------------
# Build options
#-----------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL "vneio")
    set(VNEIO_DEV_DEFAULT ON)
else()
    set(VNEIO_DEV_DEFAULT OFF)
endif()
option(VNEIO_DEV "Dev build: tests/examples defaults for local development" ${VNEIO_DEV_DEFAULT})
option(VNEIO_CI "CI build: tighten defaults for pipelines (-DVNEIO_CI=ON)" OFF)
option(VNEIO_BUILD_TESTS "Build VneIo tests" OFF)
option(VNEIO_BUILD_EXAMPLES "Build VneIo examples" OFF)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer + UndefinedBehaviorSanitizer (Clang/GCC, Linux/macOS only)" OFF)
option(VNEIO_BUILD_MESH "Build mesh component" ON)
option(VNEIO_BUILD_IMAGE "Build image component" ON)
option(VNEIO_USE_STB_IMAGE_RESIZE "Enable stb_image_resize for higher-quality resizing" OFF)
option(VNEIO_WITH_GDCM "Enable DICOM support via GDCM" OFF)
option(VNEIO_WITH_DCMTK "Enable DICOM support via DCMTK" OFF)

if(VNEIO_CI)
    set(VNEIO_DEV OFF CACHE BOOL "" FORCE)
    set(VNEIO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    message(STATUS "VneIo: CI build -> VNEIO_BUILD_EXAMPLES=OFF (set VNEIO_BUILD_TESTS in CMake args as needed)")
elseif(VNEIO_DEV)
    set(VNEIO_BUILD_TESTS ON CACHE BOOL "" FORCE)
    set(VNEIO_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
    message(STATUS "VneIo: DEV build -> VNEIO_BUILD_TESTS=ON, VNEIO_BUILD_EXAMPLES=ON")
endif()

if(ENABLE_COVERAGE AND ENABLE_ASAN)
    message(FATAL_ERROR
        "ENABLE_COVERAGE and ENABLE_ASAN cannot both be ON. "
        "Reconfigure with -DENABLE_COVERAGE=OFF or -DENABLE_ASAN=OFF.")
endif()

include(FindCoverage OPTIONAL)
if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled (gcov/lcov); compile and link flags will add --coverage")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    else()
        message(WARNING "ENABLE_ASAN is only supported with GCC/Clang on Linux/macOS — ignoring.")
    endif()
endif()

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
if(EXISTS "${VNEIO_ROOT}/cmake/vnecmake/modules/ProjectWarnings.cmake")
    include(ProjectWarnings)
    set_project_warnings(VneIoWarnings)
else()
    target_compile_options(VneIoWarnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )
endif()
add_library(vne::io::Warnings ALIAS VneIoWarnings)

add_library(VneIoBuildSettings INTERFACE)
add_library(vne::io::BuildSettings ALIAS VneIoBuildSettings)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(VneIoBuildSettings INTERFACE VNE_BUILD_TYPE_DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(VneIoBuildSettings INTERFACE VNE_BUILD_TYPE_RELEASE)
endif()

#-----------------------------------------------------------------------------
# vnelogging (from deps/internal when present)
#-----------------------------------------------------------------------------
set(VNEIO_DEPS_INTERNAL_DIR ${VNEIO_ROOT}/deps/internal)
if(EXISTS "${VNEIO_DEPS_INTERNAL_DIR}/vnelogging/CMakeLists.txt")
    if(NOT TARGET vne::common AND EXISTS "${VNEIO_DEPS_INTERNAL_DIR}/vnecommon/CMakeLists.txt")
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(deps/internal/vnecommon deps/internal/vnecommon)
    endif()
    if(NOT TARGET vne::logging)
        set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(deps/internal/vnelogging ${CMAKE_BINARY_DIR}/deps/internal/vnelogging)
        set(BUILD_TESTS ${VNEIO_BUILD_TESTS} CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES ${VNEIO_BUILD_EXAMPLES} CACHE BOOL "" FORCE)
    endif()
    set(VNEIO_HAS_LOGGING ON)
    message(STATUS "VneIo: using vnelogging from deps/internal/vnelogging")
endif()

#-----------------------------------------------------------------------------
# External / third-party: Assimp (for mesh)
# Prefer deps/external/assimp, then 3rd_party/assimp, then find_package
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_MESH)
    if(EXISTS "${VNEIO_EXTERNAL_DIR}/assimp/CMakeLists.txt")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        # Minimal Assimp build: OBJ + STL + PLY import (no exporters; we use our own OBJ exporter)
        set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_PLY_IMPORTER ON CACHE BOOL "" FORCE)
        add_subdirectory(deps/external/assimp deps/external/assimp EXCLUDE_FROM_ALL)
        set(VNEIO_ASSIMP_TARGET assimp)
        set(VNEIO_ASSIMP_INCLUDE "${VNEIO_EXTERNAL_DIR}/assimp/include")
        message(STATUS "VneIo: using Assimp from deps/external/assimp")
    elseif(EXISTS "${VNEIO_THIRD_PARTY_DIR}/assimp/CMakeLists.txt")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_PLY_IMPORTER ON CACHE BOOL "" FORCE)
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
            message(WARNING "VneIo: Assimp not found. Mesh loader will not be available. Add deps/external/assimp, 3rd_party/assimp, or set assimp_DIR.")
        endif()
    endif()
endif()

#-----------------------------------------------------------------------------
# External / third-party: stb_image (for image)
# Prefer deps/external/stb_image, then 3rd_party/stb_image, then FetchContent
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_IMAGE)
    set(VNEIO_STB_TARGET "")
    set(VNEIO_STB_INCLUDE "")
    if(EXISTS "${VNEIO_EXTERNAL_DIR}/stb_image/CMakeLists.txt")
        set(VNE_TARGET_PLATFORM ${VNE_TARGET_PLATFORM})
        add_subdirectory(deps/external/stb_image deps/external/stb_image)
        set(VNEIO_STB_TARGET stb_image)
        set(VNEIO_STB_INCLUDE "${VNEIO_EXTERNAL_DIR}/stb_image/src")
        message(STATUS "VneIo: using stb_image from deps/external/stb_image")
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
# External / third-party: nrrdio (required for NRRD volume loading)
# Prefer deps/external/nrrdio, then 3rd_party/nrrdio, then find_package
# NrrdIO optionally links ZLIB for gzip-compressed NRRD; disable if ZLIB not found (e.g. Windows CI).
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_IMAGE)
    set(VNEIO_NRRDIO_TARGET "")
    set(VNEIO_NRRDIO_INCLUDE "")
    set(VNEIO_NRRDIO_FOUND OFF)

    if(EXISTS "${VNEIO_EXTERNAL_DIR}/nrrdio/CMakeLists.txt" OR EXISTS "${VNEIO_EXTERNAL_DIR}/nrrdio/nrrdio/CMakeLists.txt")
        find_package(ZLIB QUIET)
        if(NOT ZLIB_FOUND)
            set(NRRDIO_USE_ZLIB OFF CACHE BOOL "Disable ZLIB in NrrdIO when ZLIB not found" FORCE)
            message(STATUS "VneIo: ZLIB not found; building NrrdIO without gzip support")
        endif()
        # Try nrrdio subdirectory first (if cloned as teem-nrrdio-nrrdio.git)
        if(EXISTS "${VNEIO_EXTERNAL_DIR}/nrrdio/nrrdio/CMakeLists.txt")
            add_subdirectory(deps/external/nrrdio/nrrdio deps/external/nrrdio/nrrdio EXCLUDE_FROM_ALL)
            set(VNEIO_NRRDIO_TARGET NrrdIO)
            set(VNEIO_NRRDIO_INCLUDE "${VNEIO_EXTERNAL_DIR}/nrrdio/nrrdio")
            set(VNEIO_NRRDIO_FOUND ON)
            message(STATUS "VneIo: using nrrdio from deps/external/nrrdio/nrrdio")
        elseif(EXISTS "${VNEIO_EXTERNAL_DIR}/nrrdio/CMakeLists.txt")
            add_subdirectory(deps/external/nrrdio deps/external/nrrdio EXCLUDE_FROM_ALL)
            set(VNEIO_NRRDIO_TARGET NrrdIO)
            set(VNEIO_NRRDIO_INCLUDE "${VNEIO_EXTERNAL_DIR}/nrrdio")
            set(VNEIO_NRRDIO_FOUND ON)
            message(STATUS "VneIo: using nrrdio from deps/external/nrrdio")
        endif()
    elseif(EXISTS "${VNEIO_THIRD_PARTY_DIR}/nrrdio/CMakeLists.txt")
        add_subdirectory(3rd_party/nrrdio 3rd_party/nrrdio EXCLUDE_FROM_ALL)
        set(VNEIO_NRRDIO_TARGET NrrdIO)
        set(VNEIO_NRRDIO_INCLUDE "${VNEIO_THIRD_PARTY_DIR}/nrrdio")
        set(VNEIO_NRRDIO_FOUND ON)
        message(STATUS "VneIo: using nrrdio from 3rd_party/nrrdio")
    else()
        find_package(nrrdio QUIET)
        if(nrrdio_FOUND)
            set(VNEIO_NRRDIO_TARGET nrrdio::nrrdio)
            set(VNEIO_NRRDIO_INCLUDE "")
            set(VNEIO_NRRDIO_FOUND ON)
            message(STATUS "VneIo: using nrrdio from find_package")
        else()
            set(VNEIO_NRRDIO_FOUND OFF)
            message(STATUS "VneIo: nrrdio not found.")
        endif()
    endif()
endif()

if(VNEIO_BUILD_IMAGE AND NOT VNEIO_NRRDIO_FOUND)
    message(FATAL_ERROR "VneIo: nrrdio is required for the image component. Add deps/external/nrrdio or set nrrdio_DIR.")
endif()

#-----------------------------------------------------------------------------
# Mesh library
#-----------------------------------------------------------------------------
if(VNEIO_BUILD_MESH AND VNEIO_ASSIMP_TARGET)
    add_library(vneio_mesh ${_VNEIO_LIB_KIND}
        src/vertexnova/io/mesh/assimp_loader.cpp
        src/vertexnova/io/mesh/mesh_loader_registry.cpp
        src/vertexnova/io/mesh/mesh_exporter_obj.cpp
    )
    vneio_target_apply_shared_export(vneio_mesh)
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
    add_library(vneio_image ${_VNEIO_LIB_KIND}
        src/vertexnova/io/image/image.cpp
        src/vertexnova/io/image/nrrd_loader.cpp
        src/vertexnova/io/image/mhd_loader.cpp
        src/vertexnova/io/image/stb_image_loader.cpp
        src/vertexnova/io/image/volume_exporter_nrrd.cpp
        src/vertexnova/io/image/volume_exporter_mhd.cpp
    )
    vneio_target_apply_shared_export(vneio_image)
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
    if(VNEIO_USE_STB_IMAGE_RESIZE)
        target_compile_definitions(vneio_image PRIVATE VNEIO_USE_STB_IMAGE_RESIZE)
    endif()
    target_link_libraries(vneio_image
        PUBLIC VneIoWarnings VneIoBuildSettings
    )
    if(VNEIO_STB_TARGET)
        target_link_libraries(vneio_image PRIVATE ${VNEIO_STB_TARGET})
    endif()
    target_link_libraries(vneio_image PRIVATE ${VNEIO_NRRDIO_TARGET})
    if(VNEIO_NRRDIO_INCLUDE)
        target_include_directories(vneio_image PRIVATE ${VNEIO_NRRDIO_INCLUDE})
    endif()
    add_library(vne::io::image ALIAS vneio_image)
endif()

#-----------------------------------------------------------------------------
# AssetIO registry (unified load by request; depends on image + mesh)
#-----------------------------------------------------------------------------
if(TARGET vneio_image AND TARGET vneio_mesh)
    add_library(vneio_asset_io ${_VNEIO_LIB_KIND} src/vertexnova/io/asset_io.cpp)
    vneio_target_apply_shared_export(vneio_asset_io)
    target_include_directories(vneio_asset_io
        PUBLIC
            $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(vneio_asset_io PUBLIC vneio_image vneio_mesh)
    if(TARGET vneio_dicom)
        target_link_libraries(vneio_asset_io PUBLIC vneio_dicom)
    endif()
    target_link_libraries(vneio_asset_io PUBLIC VneIoWarnings VneIoBuildSettings)
    add_library(vne::io::AssetIO ALIAS vneio_asset_io)
endif()

#-----------------------------------------------------------------------------
# DICOM library (optional)
#-----------------------------------------------------------------------------
if(VNEIO_WITH_GDCM OR VNEIO_WITH_DCMTK)
    add_library(vneio_dicom ${_VNEIO_LIB_KIND}
        src/vertexnova/io/dicom/dicom_loader_registry.cpp
    )
    vneio_target_apply_shared_export(vneio_dicom)
    set_target_properties(vneio_dicom PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    target_include_directories(vneio_dicom
        PUBLIC
            $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(vneio_dicom
        PUBLIC VneIoWarnings VneIoBuildSettings
    )
    if(VNEIO_WITH_GDCM)
        target_compile_definitions(vneio_dicom PRIVATE VNEIO_WITH_GDCM)
        # TODO: find_package(GDCM) and link
    endif()
    if(VNEIO_WITH_DCMTK)
        target_compile_definitions(vneio_dicom PRIVATE VNEIO_WITH_DCMTK)
        # TODO: find_package(DCMTK) and link
    endif()
    add_library(vne::io::dicom ALIAS vneio_dicom)
else()
    # Always build dicom_loader_registry.cpp (null loader) so the interface is available
    add_library(vneio_dicom ${_VNEIO_LIB_KIND}
        src/vertexnova/io/dicom/dicom_loader_registry.cpp
    )
    vneio_target_apply_shared_export(vneio_dicom)
    set_target_properties(vneio_dicom PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    target_include_directories(vneio_dicom
        PUBLIC
            $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(vneio_dicom
        PUBLIC VneIoWarnings VneIoBuildSettings
    )
    add_library(vne::io::dicom ALIAS vneio_dicom)
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
if(TARGET vneio_asset_io)
    target_link_libraries(vneio INTERFACE vneio_asset_io)
endif()
if(TARGET vneio_dicom)
    target_link_libraries(vneio INTERFACE vneio_dicom)
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
    # stb_image is a link dependency of vneio_image when using deps/external/stb_image; include in export
    if(TARGET stb_image)
        list(APPEND VNEIO_INSTALL_TARGETS stb_image)
    endif()
    # nrrdio is required by vneio_image; include in export when from subdirectory
    if(TARGET NrrdIO)
        list(APPEND VNEIO_INSTALL_TARGETS NrrdIO)
    endif()
endif()
if(TARGET vneio_asset_io)
    list(APPEND VNEIO_INSTALL_TARGETS vneio_asset_io)
endif()
if(TARGET vneio_dicom)
    list(APPEND VNEIO_INSTALL_TARGETS vneio_dicom)
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
