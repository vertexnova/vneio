#!/bin/bash
#==============================================================================
# VneIo macOS Build Script
#==============================================================================
# Copyright (c) 2025–2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#==============================================================================

set -e
JOBS=10
ARGS=()
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs) [[ -n "$2" && "$2" =~ ^[0-9]+$ ]] && { JOBS="$2"; shift 2; } || exit 1 ;;
        -j*) JOBS="${1#-j}"; shift ;;
        *) ARGS+=("$1"); shift ;;
    esac
done
set -- "${ARGS[@]}"

PLATFORM="Darwin"
COMPILER="clang"
usage() {
  echo "Usage: $0 [-t <build_type>] [-a <action>] [-clean] [-j <jobs>] [-xcode] [-xcode-only]"
  echo "  -t Debug|Release|...   -a configure|build|configure_and_build|test|xcode|xcode_build"
  echo "  -xcode       also generate Xcode project   -xcode-only   only generate Xcode project"
  exit 1
}

BUILD_TYPE="Debug"
ACTION="configure_and_build"
CLEAN_BUILD=false
GENERATE_XCODE=false
XCODE_ONLY=false
while [[ $# -gt 0 ]]; do
  case $1 in
    -t|--build-type) BUILD_TYPE="$2"; shift 2 ;;
    -a|--action)     ACTION="$2"; shift 2 ;;
    -clean)          CLEAN_BUILD=true; shift ;;
    -xcode)          GENERATE_XCODE=true; shift ;;
    -xcode-only)     XCODE_ONLY=true; shift ;;
    -h|--help)       usage ;;
    *) echo "Unknown: $1"; usage ;;
  esac
done

[[ "$XCODE_ONLY" == true ]] && ACTION="xcode"
[[ "$GENERATE_XCODE" == true && "$ACTION" == "configure_and_build" ]] && ACTION="xcode_build"
[[ "$ACTION" == "xcode" || "$ACTION" == "xcode_build" ]] && GENERATE_XCODE=true

COMPILER_VERSION=$(clang --version 2>/dev/null | head -n1 | awk '{print $4}' | sed 's/(.*)//' | tr -d ' ')
[[ -z "$COMPILER_VERSION" || "$COMPILER_VERSION" == "version" ]] && COMPILER_VERSION=$(clang --version 2>/dev/null | head -n1 | awk '{print $3}')
CC_PATH=$(command -v clang 2>/dev/null || true)
CXX_PATH=$(command -v clang++ 2>/dev/null || true)
[[ -z "$CC_PATH" || -z "$CXX_PATH" ]] && { echo "clang/clang++ not found"; exit 1; }

PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
if [[ "$GENERATE_XCODE" == true ]]; then
  BUILD_DIR="$PROJECT_ROOT/build/${BUILD_TYPE}/xcode-macos-$COMPILER-${COMPILER_VERSION}"
else
  BUILD_DIR="$PROJECT_ROOT/build/${BUILD_TYPE}/build-macos-$COMPILER-${COMPILER_VERSION}"
fi

if [[ "$GENERATE_XCODE" == true ]]; then
  CONFIGURE_CMD="cmake -G Xcode -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=${CC_PATH} -DCMAKE_CXX_COMPILER=${CXX_PATH} -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3 -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF $PROJECT_ROOT"
  BUILD_CMD="xcodebuild -project vneio.xcodeproj -configuration $BUILD_TYPE -parallelizeTargets -jobs $JOBS"
  TEST_CMD="xcodebuild -project vneio.xcodeproj -configuration $BUILD_TYPE -target RUN_TESTS"
else
  CONFIGURE_CMD="cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=${CC_PATH} -DCMAKE_CXX_COMPILER=${CXX_PATH} -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3 -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF $PROJECT_ROOT"
  BUILD_CMD="make -j$JOBS"
  TEST_CMD="ctest --output-on-failure"
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
    eval $BUILD_CMD
    ;;
  test)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    eval $BUILD_CMD
    eval $TEST_CMD
    ;;
  xcode)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    echo "Xcode project: $BUILD_DIR"
    echo "Open: vneio.xcodeproj"
    ;;
  xcode_build)
    $CLEAN_BUILD && clean_build || ensure_build_dir
    eval $CONFIGURE_CMD
    eval $BUILD_CMD
    ;;
  *) usage ;;
esac
echo "=== Build completed ==="
echo "Build directory: $BUILD_DIR"
