# VneIo scripts

Helper scripts for building, formatting, and documentation. Run from the **repository root**.

## Build layout

Platform scripts use CMake binary directories under **`build/<lib_type>/<build_type>/…`** (default `lib_type` is **shared**), matching CI. Example:

`build/shared/Release/build-linux-gcc-14.2.0/`

Windows (Visual Studio multi-config) uses **`build/<lib_type>/`** from the PowerShell script’s perspective, with **Debug/Release** inside that tree from the generator.

### Linux (`build_linux.sh`)

```bash
./scripts/build_linux.sh -t Debug -a configure_and_build
./scripts/build_linux.sh -c clang -j 20 -t Release
./scripts/build_linux.sh -l static -t Release -clean
./scripts/build_linux.sh -interactive
```

**Options:** `-t` build type, `-a` (`configure` | `build` | `configure_and_build` | `test`), `-c` (`gcc` | `clang`), `-l` (`static` | `shared`), `-j`, `-clean`, `-interactive`, `-h`

Clang passes **`-DENABLE_IPO=OFF`** (same rationale as CI).

### macOS (`build_macos.sh`)

```bash
./scripts/build_macos.sh -t Debug -a configure_and_build
./scripts/build_macos.sh -l static -t Release
./scripts/build_macos.sh -xcode -t Release
./scripts/build_macos.sh -xcode-only -t Release
./scripts/build_macos.sh -a xcode_build -t Debug
```

**Options:** `-t`, `-a` (adds `xcode`, `xcode_build`), `-l`, `-clean`, `-j`, `-xcode`, `-xcode-only`, `-h`

Xcode project name: **`vneio.xcodeproj`**.

### Windows

**Bash** (Git Bash): `build_windows.sh` — `cl` and `cmake` on `PATH`.

```bash
./scripts/build_windows.sh -t Debug -a configure_and_build
./scripts/build_windows.sh -j 8 -t Release -l static
```

**Python** (Developer Command Prompt): `build_windows.py`

```bash
python scripts/build_windows.py -t Debug -a configure_and_build
python scripts/build_windows.py --build-type Release --jobs 8 -l static
python scripts/build_windows.py --interactive
```

**PowerShell** (`build_windows.ps1`):

```powershell
.\scripts\build_windows.ps1 -BuildType Release -LibType shared -Action test
```

### Web — Emscripten (`build_web.sh`)

```bash
./scripts/build_web.sh Release
./scripts/build_web.sh -c -j 8 Debug
```

Defaults: **`build/shared/<type>/…`**, **`VNEIO_BUILD_MESH=OFF`** (CI-style), **`VNEIO_BUILD_TESTS=OFF`**. Override lib type: `-l static`.

### iOS (`build_ios.sh`)

```bash
./scripts/build_ios.sh -t Release -a configure_and_build -simulator
./scripts/build_ios.sh -l shared -device -deployment-target 16.0
```

Defaults: **`lib_type=static`**, **`build/static/<type>/…`**, **`VNEIO_BUILD_TESTS=OFF`**.

## Formatting (`clang_formatter.py`)

Uses **`clang-format-17`** when available (same as CI), else `clang-format`. **`--dry-run`** exits non-zero if anything would change.

```bash
python3 scripts/clang_formatter.py all --dry-run
python3 scripts/clang_formatter.py src include tests
python3 scripts/clang_formatter.py --file src/vertexnova/io/asset_io.cpp
```

`all` scans **`src`**, **`include`**, **`examples`** (if present), **`tests`**.

## Documentation (`generate-docs.sh`)

- **API:** runs only if **`docs/doxyfile.in`** exists and CMake exposes a Doxygen target (e.g. after adding `ENABLE_DOXYGEN` to the project).
- Otherwise: link check (optional **`markdown-link-check`**) + a simple tag heuristic on `include/` and `src/`.

```bash
./scripts/generate-docs.sh
./scripts/generate-docs.sh --api-only
./scripts/generate-docs.sh --validate
```

Uses configure tree **`build/shared/Release`** for CMake-driven API generation.

## Prerequisites

- CMake **3.16+**, C++20 compiler  
- Mesh: Assimp (`deps/external/assimp` or system, e.g. `libassimp-dev`)  
- Image: stb + nrrdio per main **README**  
- Submodules: `git submodule update --init --recursive`
