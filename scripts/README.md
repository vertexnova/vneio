# VneIo Scripts

Build and platform scripts for the VneIo library (mesh + image).

| Script              | Purpose                                      |
|---------------------|----------------------------------------------|
| `build_linux.sh`    | Build on Linux (GCC/Clang)                   |
| `build_macos.sh`    | Build on macOS (Clang/Xcode)                 |
| `build_windows.py`  | Build on Windows (MSVC)                      |
| `build_web.sh`      | Build for Web (Emscripten)                   |
| `build_ios.sh`      | Build for iOS (simulator/device)             |

## Quick usage

```bash
# Linux
./scripts/build_linux.sh -t Debug -a configure_and_build

# macOS (Makefiles)
./scripts/build_macos.sh -t Debug -a configure_and_build

# macOS (Xcode project)
./scripts/build_macos.sh -xcode-only -t Debug
# then open build/Debug/xcode-macos-clang-*/vneio.xcodeproj

# Windows (from Developer Command Prompt)
python scripts/build_windows.py -t Debug -a configure_and_build

# Web (requires Emscripten)
./scripts/build_web.sh Debug

# iOS
./scripts/build_ios.sh -t Debug -a configure_and_build -simulator
```

## Options (common)

- `-t, --build-type` — Debug, Release, RelWithDebInfo, MinSizeRel
- `-a, --action` — configure, build, configure_and_build, test
- `-clean` — remove build directory before running
- `-j <N>` — parallel jobs (where supported)

macOS: `-xcode` (also generate Xcode project), `-xcode-only` (only generate Xcode project).
iOS: `-simulator` (default), `-device`, `-deployment-target <version>`.

## Prerequisites

- CMake 3.16+
- C++20-capable compiler
- For mesh: Assimp (via `deps/external/assimp` or system)
- For image: stb_image (via `deps/external/stb_image` or FetchContent)
- Optional logging: `deps/internal/vnecommon`, `deps/internal/vnelogging` (submodules)
- **Tests:** minimal testdata is in `testdata/` (no LFS).

Clone with submodules: `git submodule update --init --recursive`
