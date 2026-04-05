#==============================================================================
# Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   April 2026
#
#==============================================================================

include_guard(GLOBAL)

option(GIT_SUBMODULE "Run git submodule update --init --recursive and verify VneIo submodule trees" ON)

if(NOT GIT_SUBMODULE)
    return()
endif()

find_package(Git QUIET)
if(NOT GIT_FOUND)
    message(STATUS "GitSubmodule: Git not found — skipping submodule update")
    return()
endif()

get_filename_component(_VNEIO_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT EXISTS "${_VNEIO_ROOT_DIR}/.git")
    message(STATUS "GitSubmodule: not a git checkout — skipping submodule update")
    return()
endif()

message(STATUS "GitSubmodule: git submodule update --init --recursive")
execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
    WORKING_DIRECTORY "${_VNEIO_ROOT_DIR}"
    RESULT_VARIABLE _VNEIO_SUBMODULE_RESULT
)
if(NOT _VNEIO_SUBMODULE_RESULT EQUAL "0")
    message(FATAL_ERROR
        "git submodule update --init --recursive failed (${_VNEIO_SUBMODULE_RESULT}). "
        "Fix your clone or pass -DGIT_SUBMODULE=OFF if dependencies are provided elsewhere (e.g. 3rd_party).")
endif()

# Registered in .gitmodules — expected after a successful update
set(_VNEIO_SUBMODULE_MARKERS
    "${_VNEIO_ROOT_DIR}/cmake/vnecmake/modules/ProjectWarnings.cmake"
    "${_VNEIO_ROOT_DIR}/deps/internal/vnecommon/CMakeLists.txt"
    "${_VNEIO_ROOT_DIR}/deps/internal/vnelogging/CMakeLists.txt"
    "${_VNEIO_ROOT_DIR}/deps/external/assimp/CMakeLists.txt"
    "${_VNEIO_ROOT_DIR}/deps/external/googletest/CMakeLists.txt"
)
foreach(_VNEIO_SM ${_VNEIO_SUBMODULE_MARKERS})
    if(NOT EXISTS "${_VNEIO_SM}")
        message(FATAL_ERROR
            "VneIo submodule tree incomplete — missing:\n  ${_VNEIO_SM}\n"
            "Run from the repo root: git submodule update --init --recursive\n"
            "Or configure with -DGIT_SUBMODULE=OFF if you use 3rd_party/ or other layouts.")
    endif()
endforeach()

# nrrdio: submodule may be flat (CMakeLists.txt at root) or nested (nrrdio/nrrdio)
if(NOT EXISTS "${_VNEIO_ROOT_DIR}/deps/external/nrrdio/CMakeLists.txt"
    AND NOT EXISTS "${_VNEIO_ROOT_DIR}/deps/external/nrrdio/nrrdio/CMakeLists.txt")
    message(FATAL_ERROR
        "VneIo nrrdio submodule missing. Expected:\n"
        "  deps/external/nrrdio/CMakeLists.txt\n"
        "or\n"
        "  deps/external/nrrdio/nrrdio/CMakeLists.txt\n"
        "Run: git submodule update --init --recursive\n"
        "Or use -DGIT_SUBMODULE=OFF with nrrdio supplied under 3rd_party/ or install tree.")
endif()
