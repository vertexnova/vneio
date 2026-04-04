# =============================================================================
# Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   April 2026
#
# VneIo examples — headless asset I/O demos (no window, no renderer).
# Default: OFF. Local dev: -DVNEIO_DEV=ON turns examples ON. CI/coverage: OFF.
# Manual: cmake -B build -DVNEIO_BUILD_EXAMPLES=ON (requires VNEIO_CI=OFF or no FORCE from coverage).
# =============================================================================

cmake_minimum_required(VERSION 3.16)
project(VneIoExamples)

# Disable on mobile / web
if(DEFINED VNE_TARGET_PLATFORM AND
   (VNE_TARGET_PLATFORM STREQUAL "iOS" OR VNE_TARGET_PLATFORM STREQUAL "Web"))
    message(STATUS "VneIo examples disabled for ${VNE_TARGET_PLATFORM}")
    return()
endif()

if(NOT TARGET vneio_asset_io)
    message(STATUS "VneIo examples require vneio_asset_io; skipping.")
    return()
endif()

set(VNEIO_EXAMPLES_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

# Logging include path (embedded in library; examples only need the headers)
set(_VNEIO_EX_LOGGING_INCLUDES "")
if(DEFINED VNEIO_DEPS_INTERNAL_DIR AND
   EXISTS "${VNEIO_DEPS_INTERNAL_DIR}/vnelogging/include")
    list(APPEND _VNEIO_EX_LOGGING_INCLUDES
        "${VNEIO_DEPS_INTERNAL_DIR}/vnelogging/include"
        "${VNEIO_DEPS_INTERNAL_DIR}/vnelogging/src"
        "${CMAKE_BINARY_DIR}/deps/internal/vnelogging"
    )
    message(STATUS "VneIo examples: vnelogging headers found at ${VNEIO_DEPS_INTERNAL_DIR}/vnelogging/include")
else()
    message(STATUS "VneIo examples: vnelogging headers not found — logging output will fall back to stdout")
endif()

# -----------------------------------------------------------------------------
# Helper macro — one line per example in subdirectory CMakeLists files.
# Usage: vne_add_example(<TargetName> main.cpp XX_example.cpp)
# -----------------------------------------------------------------------------
macro(vne_add_example target_name)
    add_executable(${target_name} ${ARGN})
    target_include_directories(${target_name} PRIVATE
        $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<BUILD_INTERFACE:${VNEIO_EXAMPLES_COMMON_DIR}>
    )
    if(_VNEIO_EX_LOGGING_INCLUDES)
        target_include_directories(${target_name} PRIVATE ${_VNEIO_EX_LOGGING_INCLUDES})
        target_compile_definitions(${target_name} PRIVATE VNEIO_EXAMPLES_HAS_LOGGING=1)
        # Shared libvneio_* links vnelogging privately; only symbols referenced from mesh/image
        # object files end up in the dylib. LoggingGuard needs Logging::configureLogger etc.,
        # so link vnelogging into the executable for shared VneIo builds only (static embeds
        # already include those symbols in libvneio_*).
        if(TARGET vne::logging AND DEFINED VNEIO_LIB_TYPE AND VNEIO_LIB_TYPE STREQUAL "shared")
            target_link_libraries(${target_name} PRIVATE vne::logging)
        endif()
    endif()
    target_link_libraries(${target_name} PRIVATE vneio_asset_io)
    target_compile_definitions(${target_name} PRIVATE
        "VNEIO_TESTDATA_DIR=\"${VNEIO_TESTDATA_DIR}\""
    )
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
    )
endmacro()

add_subdirectory(01_library_info)
add_subdirectory(02_image_loading)
add_subdirectory(03_volume_loading)
add_subdirectory(04_volume_export)
add_subdirectory(05_mesh_loading)
add_subdirectory(06_asset_registry)
