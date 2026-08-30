#==============================================================================
# Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   January 2026
#
# VneIo tests - layout and Google Test setup follow vnemath/tests
#==============================================================================

cmake_minimum_required(VERSION 3.16)

project(TestVneIo)

set(_VNEIO_TESTS_CONFIGURE TRUE)
# Disable tests for iOS and Web
if(DEFINED VNE_TARGET_PLATFORM AND (VNE_TARGET_PLATFORM STREQUAL "iOS" OR VNE_TARGET_PLATFORM STREQUAL "Web"))
    message(STATUS "VneIo tests disabled for ${VNE_TARGET_PLATFORM}")
    set(_VNEIO_TESTS_CONFIGURE FALSE)
endif()
# Require both mesh and image components
if(NOT TARGET vneio_mesh OR NOT TARGET vneio_image)
    message(STATUS "VneIo tests require VNEIO_BUILD_MESH and VNEIO_BUILD_IMAGE; skipping.")
    set(_VNEIO_TESTS_CONFIGURE FALSE)
endif()

if(_VNEIO_TESTS_CONFIGURE)
#-----------------------------------------------------------------------------
# Google Test (deps/external submodule, stable v1.14.0)
#-----------------------------------------------------------------------------
if(EXISTS "${VNEIO_ROOT}/deps/external/googletest/CMakeLists.txt")
    message(STATUS "Using Google Test from deps/external/googletest (v1.14.0)")
    add_subdirectory(${VNEIO_ROOT}/deps/external/googletest ${CMAKE_BINARY_DIR}/deps/external/googletest)
    include(GoogleTest)
    set(GTEST_LIBRARIES gtest_main gmock)
    set(GTEST_INCLUDE_DIRS
        ${VNEIO_ROOT}/deps/external/googletest/googletest/include
        ${VNEIO_ROOT}/deps/external/googletest/googlemock/include
    )
else()
    find_package(GTest QUIET)
    if(GTest_FOUND)
        message(STATUS "Using system Google Test")
        set(GTEST_LIBRARIES GTest::gtest_main GTest::gmock)
        set(GTEST_INCLUDE_DIRS "")
    else()
        message(STATUS "deps/external/googletest not found, using FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
        )
        FetchContent_MakeAvailable(googletest)
        set(GTEST_LIBRARIES gtest_main gmock)
        set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include ${googletest_SOURCE_DIR}/googlemock/include)
    endif()
endif()

if(COMMAND vneio_target_suppress_external_warnings)
    foreach(_vneio_ext_gtest IN ITEMS gtest gtest_main gmock)
        if(TARGET "${_vneio_ext_gtest}")
            vneio_target_suppress_external_warnings("${_vneio_ext_gtest}")
        endif()
    endforeach()
endif()

#-----------------------------------------------------------------------------
# Test sources (mesh + image, from vertexnova/tests/core)
#-----------------------------------------------------------------------------
set(TEST_SOURCES
    asset_io_test.cpp
    mesh/mesh_loader_test.cpp
    image/image_test.cpp
    image/volume_test.cpp
    main.cpp
)
# path_utils (needs vneio_config.h from build dir)
list(APPEND TEST_SOURCES ${VNEIO_ROOT}/src/vertexnova/io/utils/path_utils.cpp)

add_executable(TestVneIo ${TEST_SOURCES})

# Shared DLL + tests: consumers see VNEIO_DLL / dllimport; MSVC C4251 on STL in exported
# classes is expected (same as vneio targets' /wd4251 /wd4275). Match actual vneio_asset_io TYPE.
if(MSVC AND TARGET vneio_asset_io)
    get_target_property(_VNEIO_TEST_ASSET_IO_TYPE vneio_asset_io TYPE)
    if(_VNEIO_TEST_ASSET_IO_TYPE STREQUAL "SHARED_LIBRARY")
        target_compile_options(TestVneIo PRIVATE /wd4251 /wd4275)
    endif()
endif()

target_include_directories(TestVneIo
    PUBLIC
        $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)
if(GTEST_INCLUDE_DIRS)
    target_include_directories(TestVneIo SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
endif()

# Link asset_io (it PUBLIC-depends on vneio_mesh and vneio_image, so no need to list them again)
set(VNEIO_TEST_LIBS "")
if(TARGET vneio_asset_io)
    list(APPEND VNEIO_TEST_LIBS vneio_asset_io)
endif()
# NrrdIO OBJECT sources are merged into vneio_image; linking NrrdIO again duplicates symbols (e.g. main in sampleIO.c).
if(VNEIO_NRRDIO_TARGET)
    get_target_property(_vneio_test_nrrd_type "${VNEIO_NRRDIO_TARGET}" TYPE)
    get_target_property(_vneio_test_nrrd_imported "${VNEIO_NRRDIO_TARGET}" IMPORTED)
    if(_vneio_test_nrrd_imported OR NOT _vneio_test_nrrd_type STREQUAL "OBJECT_LIBRARY")
        list(APPEND VNEIO_TEST_LIBS ${VNEIO_NRRDIO_TARGET})
    endif()
endif()

target_link_libraries(TestVneIo
    PRIVATE
        ${GTEST_LIBRARIES}
        ${VNEIO_TEST_LIBS}
)

# Ensure testdata root is defined (config.h or fallback from CMake)
target_compile_definitions(TestVneIo PRIVATE "VNEIO_TESTDATA_DIR=\"${VNEIO_TESTDATA_DIR}\"")
if(VNEIO_BUILD_USD)
    target_compile_definitions(TestVneIo PRIVATE VNEIO_HAS_USD=1)
endif()

# Run from project root so testdata paths resolve; testdata root is absolute from config
add_test(NAME vneio.test COMMAND TestVneIo)
set_tests_properties(vneio.test PROPERTIES WORKING_DIRECTORY ${VNEIO_ROOT})

endif()
