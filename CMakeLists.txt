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
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(VNEIO_BUILD_MESH "Build mesh component" ON)
option(VNEIO_BUILD_IMAGE "Build image component" ON)
option(VNEIO_USE_STB_IMAGE_RESIZE "Enable stb_image_resize for higher-quality resizing" OFF)
option(VNEIO_WITH_GDCM "Enable DICOM support via GDCM" OFF)
option(VNEIO_WITH_DCMTK "Enable DICOM support via DCMTK" OFF)

# Enable coverage (uses FindCoverage from cmake/vnecmake)
include(FindCoverage)

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
        # Minimal Assimp build: OBJ + STL import only (no exporters; we use our own OBJ exporter)
        set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "" FORCE)
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
    add_library(vneio_mesh STATIC
        src/vertexnova/io/mesh/assimp_loader.cpp
        src/vertexnova/io/mesh/mesh_loader_registry.cpp
        src/vertexnova/io/mesh/mesh_exporter_obj.cpp
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
        src/vertexnova/io/image/nrrd_loader.cpp
        src/vertexnova/io/image/mhd_loader.cpp
        src/vertexnova/io/image/stb_image_loader.cpp
        src/vertexnova/io/image/volume_exporter_nrrd.cpp
        src/vertexnova/io/image/volume_exporter_mhd.cpp
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
    add_library(vneio_asset_io STATIC src/vertexnova/io/asset_io.cpp)
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
    add_library(vneio_dicom STATIC
        src/vertexnova/io/dicom/dicom_loader_registry.cpp
    )
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
    add_library(vneio_dicom STATIC
        src/vertexnova/io/dicom/dicom_loader_registry.cpp
    )
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
