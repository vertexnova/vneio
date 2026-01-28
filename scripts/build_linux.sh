#!/bin/bash
#==============================================================================
# VneIo Linux Build Script
#==============================================================================
# Copyright (c) 2025–2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#==============================================================================

set -e
JOBS=10
ARGS=()
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs) [[ -n "$2" && "$2" =~ ^[0-9]+$ ]] && { JOBS="$2"; shift 2; } || { echo "Invalid jobs"; exit 1; } ;;
        -j*) JOBS="${1#-j}"; shift ;;
        *) ARGS+=("$1"); shift ;;
    esac
done
set -- "${ARGS[@]}"

PLATFORM="Linux"
COMPILER="gcc"
usage() {
  echo "Usage: $0 [-t <build_type>] [-a <action>] [-c <compiler>] [-clean] [-j <jobs>]"
  echo "  -t Debug|Release|RelWithDebInfo|MinSizeRel"
  echo "  -a configure|build|configure_and_build|test"
  echo "  -c gcc|clang    -j <N>    -clean"
  exit 1
}

BUILD_TYPE="Debug"
ACTION="configure_and_build"
CLEAN_BUILD=false
while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--build-type) BUILD_TYPE="$2"; shift 2 ;;
    -a|--action)     ACTION="$2"; shift 2 ;;
    -c|--compiler)   COMPILER="$2"; shift 2 ;;
    -clean)          CLEAN_BUILD=true; shift ;;
    -h|--help)       usage ;;
    *) echo "Unknown: $1"; usage ;;
  esac
done

if [[ "$COMPILER" == "gcc" ]]; then
  COMPILER_VERSION=$(gcc --version | head -n1 | awk '{print $4}')
elif [[ "$COMPILER" == "clang" ]]; then
  COMPILER_VERSION=$(clang --version | head -n1 | awk '{print $3}')
else
  echo "Unsupported compiler: $COMPILER"; exit 1
fi

PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build/${BUILD_TYPE}/build-linux-$COMPILER-${COMPILER_VERSION}"

if [[ "$COMPILER" == "gcc" ]]; then
  CONFIGURE_CMD="cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DBUILD_TESTS=ON $PROJECT_ROOT"
else
  CONFIGURE_CMD="cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_TESTS=ON $PROJECT_ROOT"
fi

clean_build() { rm -rf "$BUILD_DIR"; mkdir -p "$BUILD_DIR"; cd "$BUILD_DIR" || exit; }
ensure_build_dir() { mkdir -p "$BUILD_DIR"; cd "$BUILD_DIR" || exit; }

case $ACTION in
  configure)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    ;;
  build|configure_and_build)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    make -j$JOBS
    ;;
  test)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    make -j$JOBS
    ctest --output-on-failure
    ;;
  *) usage ;;
esac
echo "=== Build completed ==="
echo "Build directory: $BUILD_DIR"
