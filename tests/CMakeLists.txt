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

# Disable tests for iOS and Web
if(DEFINED VNE_TARGET_PLATFORM AND (VNE_TARGET_PLATFORM STREQUAL "iOS" OR VNE_TARGET_PLATFORM STREQUAL "Web"))
    message(STATUS "VneIo tests disabled for ${VNE_TARGET_PLATFORM}")
    return()
endif()

# Require both mesh and image components
if(NOT TARGET vneio_mesh OR NOT TARGET vneio_image)
    message(STATUS "VneIo tests require VNEIO_BUILD_MESH and VNEIO_BUILD_IMAGE; skipping.")
    return()
endif()

#-----------------------------------------------------------------------------
# Google Test (external submodule, stable v1.14.0)
#-----------------------------------------------------------------------------
if(EXISTS "${VNEIO_ROOT}/external/googletest/CMakeLists.txt")
    message(STATUS "Using Google Test from external/googletest (v1.14.0)")
    add_subdirectory(${VNEIO_ROOT}/external/googletest ${CMAKE_BINARY_DIR}/external/googletest)
    include(GoogleTest)
    set(GTEST_LIBRARIES gtest_main gmock)
    set(GTEST_INCLUDE_DIRS
        ${VNEIO_ROOT}/external/googletest/googletest/include
        ${VNEIO_ROOT}/external/googletest/googlemock/include
    )
else()
    find_package(GTest QUIET)
    if(GTest_FOUND)
        message(STATUS "Using system Google Test")
        set(GTEST_LIBRARIES GTest::gtest_main GTest::gmock)
        set(GTEST_INCLUDE_DIRS "")
    else()
        message(STATUS "external/googletest not found, using FetchContent")
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

#-----------------------------------------------------------------------------
# Test sources (mesh + image, from vertexnova/tests/core)
#-----------------------------------------------------------------------------
set(TEST_SOURCES
    mesh/mesh_loader_test.cpp
    image/image_test.cpp
    main.cpp
)

add_executable(TestVneIo ${TEST_SOURCES})

target_include_directories(TestVneIo
    PUBLIC
        $<BUILD_INTERFACE:${VNEIO_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VNEIO_SRC_DIR}>
)
if(GTEST_INCLUDE_DIRS)
    target_include_directories(TestVneIo SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
endif()

# Link mesh and image libs when built
set(VNEIO_TEST_LIBS "")
if(TARGET vneio_mesh)
    list(APPEND VNEIO_TEST_LIBS vneio_mesh)
endif()
if(TARGET vneio_image)
    list(APPEND VNEIO_TEST_LIBS vneio_image)
endif()

target_link_libraries(TestVneIo
    PRIVATE
        ${GTEST_LIBRARIES}
        ${VNEIO_TEST_LIBS}
)

# Run from project root so testdata/vneresources/resources/... paths work
add_test(NAME vneio.test COMMAND TestVneIo)
set_tests_properties(vneio.test PROPERTIES WORKING_DIRECTORY ${VNEIO_ROOT})
