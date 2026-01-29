#==============================================================================
# Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   JUNE-2025
#
# Autodoc:   yes
#==============================================================================

# Minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Project declaration
project(stb_image)

#==============================================================================
#                              Source and Headers                              #
#==============================================================================

# Define header files
set(INCLUDE_FILES
    src/stb_image.h
    src/stb_image_write.h
    src/stb_image_resize2.h
)

# Define source files based on the selected APIs
set(SOURCES_FILES
    src/stb_image.cpp
    src/stb_image_write.cpp
    src/stb_image_resize2.cpp
)


#==============================================================================
#                                 Library                                      #
#==============================================================================

# Create a library from the selected source and header files
add_library(stb_image STATIC ${INCLUDE_FILES} ${SOURCES_FILES})

# Set include directories for the library
target_include_directories(stb_image
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Platform-specific options
if(EMSCRIPTEN)
    # Web build options (Emscripten)
    target_compile_definitions(stb_image PRIVATE STBI_NO_SIMD)
elseif(${VNE_TARGET_PLATFORM} STREQUAL "Android" OR ${VNE_TARGET_PLATFORM} STREQUAL "iOS")
    # Mobile platform optimizations
    target_compile_definitions(stb_image PRIVATE STBI_NO_SIMD)
endif()

# Set properties for multiplatform
set_target_properties(stb_image PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
