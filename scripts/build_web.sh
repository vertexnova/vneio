#!/bin/bash
#==============================================================================
# VneIo Web Build Script (Emscripten)
#==============================================================================
# Copyright (c) 2025–2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#==============================================================================

set -e
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

get_cpu_cores() {
  [[ "$OSTYPE" == "darwin"* ]] && sysctl -n hw.ncpu
  [[ "$OSTYPE" == "linux-gnu"* ]] && nproc
  [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]] && nproc
  echo 4
}

BUILD_TYPE="Debug"
CLEAN_BUILD=false
FORCE_REBUILD=false
JOBS=$(get_cpu_cores)

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean)   CLEAN_BUILD=true; shift ;;
    -f|--force)   FORCE_REBUILD=true; CLEAN_BUILD=true; shift ;;
    -j|--jobs)    [[ -n "$2" && "$2" =~ ^[0-9]+$ ]] && { JOBS="$2"; shift 2; } || { print_error "Invalid jobs"; exit 1; } ;;
    -h|--help)    echo "Usage: $0 [-c] [-f] [-j N] [Debug|Release|...]"; exit 0 ;;
    Debug|Release|RelWithDebInfo|MinSizeRel) BUILD_TYPE="$1"; shift ;;
    *) print_error "Unknown: $1"; exit 1 ;;
  esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
print_status "VneIo Web Build — $PROJECT_ROOT — $BUILD_TYPE — jobs $JOBS"

[[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]] && { print_error "CMakeLists.txt not found"; exit 1; }

EMSDK_DIR=""
[[ -d "$PROJECT_ROOT/deps/external/emsdk" ]] && EMSDK_DIR="$PROJECT_ROOT/deps/external/emsdk"
[[ -d "$HOME/emsdk" ]] && EMSDK_DIR="$HOME/emsdk"
[[ -n "$EMSDK" ]] && EMSDK_DIR="$EMSDK"
[[ -z "$EMSDK_DIR" ]] && { print_error "Emscripten SDK not found. Install: git clone https://github.com/emscripten-core/emsdk.git ~/emsdk && cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest"; exit 1; }

if [[ -f "$EMSDK_DIR/emsdk_env.sh" ]]; then
  source "$EMSDK_DIR/emsdk_env.sh"
else
  print_error "emsdk_env.sh not found"; exit 1
fi

command -v emcc &>/dev/null || { print_error "emcc not in PATH"; exit 1; }
EMCC_VERSION=$(emcc --version | head -n1)
EMCC_VERSION_NUM=$(echo "$EMCC_VERSION" | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+' | head -n1)
[[ -z "$EMCC_VERSION_NUM" ]] && EMCC_VERSION_NUM="unknown"

BUILD_DIR="$PROJECT_ROOT/build/${BUILD_TYPE}/build-web-emscripten-${EMCC_VERSION_NUM}"
$CLEAN_BUILD && [[ -d "$BUILD_DIR" ]] && { rm -rf "$BUILD_DIR"; print_success "Cleaned"; }
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [[ "$FORCE_REBUILD" == true || ! -f CMakeCache.txt || ! grep -q "EMSCRIPTEN" CMakeCache.txt 2>/dev/null ]]; then
  [[ -f CMakeCache.txt ]] && rm -f CMakeCache.txt
  print_status "Configuring for Emscripten..."
  cmake "$PROJECT_ROOT" \
    -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE="$EMSDK_DIR/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_TESTS=ON
fi

print_status "Building..."
make -j"$JOBS"
print_success "Build completed: $BUILD_DIR"
