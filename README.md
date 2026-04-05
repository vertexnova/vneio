<p align="center">
  <img src="icons/vertexnova_logo_medallion_with_text.svg" alt="VertexNova I/O" width="320"/>
</p>

<p align="center">
  <strong>Mesh, image, volume, and DICOM I/O library for the VertexNova ecosystem</strong>
</p>

<p align="center">
  <a href="https://github.com/vertexnova/vneio/actions/workflows/ci.yml">
    <img src="https://github.com/vertexnova/vneio/actions/workflows/ci.yml/badge.svg?branch=main" alt="CI"/>
  </a>
  <a href="https://codecov.io/gh/vertexnova/vneio">
    <img src="https://codecov.io/gh/vertexnova/vneio/branch/main/graph/badge.svg" alt="Coverage"/>
  </a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++ Standard"/>
  <img src="https://img.shields.io/badge/license-Apache%202.0-green.svg" alt="License"/>
</p>

---

## About

**vneio** is a C++20 asset I/O library extracted from VertexNova core. It does **not** provide rendering, windowing, or GPU state — your application loads files and receives plain C++ structs (`Mesh`, `Image`, `Volume`, `DicomSeries`) ready for upload or further processing.

It is independent of **vnescene**, **vnemath**, and **vneevents**. **vnelogging** and **vnecommon** are optional; they follow the same `deps/internal` layout as other VertexNova repos.

## Features

- **Mesh** — `AssimpLoader` wraps the [Assimp](https://github.com/assimp/assimp) import library; supports OBJ, STL, PLY, FBX, glTF 2.0, Collada, and more. `AssimpLoaderOptions` controls triangulation, normal/tangent generation, UV flip, barycentrics, and normalization. OBJ export included.
- **Image** — `StbImageLoader` and the `Image` class wrap [stb_image](https://github.com/nothings/stb); supports PNG, JPG, BMP, TGA, HDR. `Image` provides resize, flip, and raw pixel access for GPU upload.
- **Volume** — `NrrdLoader` and `MhdLoader` implement `IVolumeLoader` using [Teem NrrdIO](https://teem.sourceforge.net/nrrd/lib.html); loads 3D voxel data with full spatial metadata (dims, spacing, origin, direction matrix, pixel type). NRRD export and MHD export included.
- **DICOM** — `DicomLoaderRegistry` with optional GDCM or DCMTK backends (`-DVNEIO_WITH_GDCM=ON` / `-DVNEIO_WITH_DCMTK=ON`). Returns `DicomSeries` (Volume + metadata map).
- **Unified registry** — `AssetIO` registers any combination of loaders and routes `LoadRequest` by asset type and file extension.
- **Stable error model** — Every call returns `LoadResult<T>` with `Status` (stable `ErrorCode` enum, message, path, subsystem). No exceptions.
- **Cross-platform** — Linux, macOS, Windows; mobile and Web follow vnescene / vnemath toolchains where those targets are enabled.

System, class, component, and runtime pipeline diagrams live in [Architecture & usage](docs/vertexnova/io/vneio.md) (sources under [`docs/vertexnova/io/diagrams/`](docs/vertexnova/io/diagrams/)).

## Installation

### Option 1: Git Submodule (Recommended)

```bash
git submodule add https://github.com/vertexnova/vneio.git deps/vneio
# Ensure Assimp, stb_image, and NrrdIO are available (see deps/external/ layout).
```

In your `CMakeLists.txt`:

```cmake
add_subdirectory(deps/vneio)
target_link_libraries(your_target PRIVATE vne::io)
# Or selectively: vne::io::mesh, vne::io::image
```

### Option 2: FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    vneio
    GIT_REPOSITORY https://github.com/vertexnova/vneio.git
    GIT_TAG main
)
set(VNEIO_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(vneio)
target_link_libraries(your_target PRIVATE vne::io)
```

### Option 3: System Install

```bash
git clone --recursive https://github.com/vertexnova/vneio.git
cd vneio
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
cmake --install build --component vneio
```

## Building

```bash
git clone --recursive https://github.com/vertexnova/vneio.git
cd vneio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For local development (examples + tests enabled):

```bash
cmake -B build -DVNEIO_DEV=ON
cmake --build build
```

Helper scripts (Linux, macOS, Windows): see [scripts/README.md](scripts/README.md).

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `VNEIO_BUILD_MESH` | `ON` | Build mesh component (requires Assimp) |
| `VNEIO_BUILD_IMAGE` | `ON` | Build image + volume component (stb_image + NrrdIO) |
| `VNEIO_BUILD_TESTS` | `OFF` | Build unit tests (GoogleTest) |
| `VNEIO_BUILD_EXAMPLES` | `OFF` | Build headless example programs |
| `VNEIO_DEV` | `ON` (top-level) | Dev preset: tests and examples ON |
| `VNEIO_CI` | `OFF` | CI preset: tests ON, examples OFF |
| `VNEIO_LIB_TYPE` | `static` | Library type: `static` or `shared` |
| `VNEIO_WITH_GDCM` | `OFF` | Enable GDCM DICOM backend |
| `VNEIO_WITH_DCMTK` | `OFF` | Enable DCMTK DICOM backend |
| `VNEIO_USE_STB_IMAGE_RESIZE` | `OFF` | stb_image_resize for quality resize |
| `ENABLE_DOXYGEN` | `OFF` | Build API documentation (Doxygen) |
| `ENABLE_COVERAGE` | `OFF` | Enable code coverage reporting |
| `ENABLE_ASAN` | `OFF` | AddressSanitizer + UBSan (GCC/Clang, Linux/macOS) |

## Library type

Default is **`static`** (`VNEIO_LIB_TYPE=static`). Use **`shared`** for a separate dylib/DLL. In static mode, `vnelogging` is embedded in the archive when the submodule is present.

## Quick Start

### Mesh

```cpp
#include <vertexnova/io/mesh/assimp_loader.h>

vne::mesh::AssimpLoader loader;
vne::mesh::Mesh mesh;
if (loader.loadFile("model.obj", mesh)) {
    // mesh.vertices, mesh.indices, mesh.parts, mesh.materials
    // mesh.has_normals, mesh.has_uv0, mesh.aabb_min, mesh.aabb_max
}
```

### Image

```cpp
#include <vertexnova/io/image/image.h>

vne::image::Image img("texture.png");
if (!img.isEmpty()) {
    int w = img.getWidth(), h = img.getHeight();
    const uint8_t* data = img.getData(); // ready for GPU upload
}
```

### Volume

```cpp
#include <vertexnova/io/image/nrrd_loader.h>

vne::image::NrrdLoader loader;
auto result = loader.loadVolume(vne::io::LoadRequest{vne::io::AssetType::eVolume, "scan.nrrd"});
if (result.ok()) {
    const auto& vol = result.value;
    float voxel = vol.readVoxelAt<float>(x, y, z);
}
```

### Unified registry

```cpp
#include <vertexnova/io/vneio.h>

vne::io::AssetIO io;
io.registerMeshLoader(std::make_shared<vne::mesh::AssimpLoader>());
io.registerImageLoader(std::make_shared<vne::image::StbImageLoader>());
io.registerVolumeLoader(std::make_shared<vne::image::NrrdLoader>());

auto mesh  = io.loadMesh(vne::io::LoadRequest{vne::io::AssetType::eMesh,  "robot.glb"});
auto image = io.loadImage(vne::io::LoadRequest{vne::io::AssetType::eImage, "albedo.png"});
```

See [examples/01_library_info](examples/01_library_info) for format and capability listing, and [examples/README.md](examples/README.md) for the full numbered index.

## Examples (headless)

All examples are **headless** (no window or GPU required). Each folder has its own `main.cpp` and a shared header.

| Example | Description |
|---------|-------------|
| [01_library_info](examples/01_library_info) | Enumerate supported formats, pixel types, and error codes |
| [02_image_loading](examples/02_image_loading) | Load, inspect, resize, and save 2D images |
| [03_volume_loading](examples/03_volume_loading) | Load NRRD/MHD volumes and inspect metadata |
| [04_volume_export](examples/04_volume_export) | Save volumes to NRRD and MHD formats |
| [05_mesh_loading](examples/05_mesh_loading) | Load meshes, inspect geometry, export OBJ, round-trip |
| [06_asset_registry](examples/06_asset_registry) | Multi-loader `AssetIO` registry pattern |
| [07_performance](examples/07_performance) | Benchmarking load times across formats |

Build examples with:

```bash
cmake -B build -DVNEIO_BUILD_EXAMPLES=ON
cmake --build build
```

## Documentation

- [Architecture & usage](docs/vertexnova/io/vneio.md) — Module design, diagrams, integration, and build configuration.
- [API documentation](docs/README.md) — Doxygen (`ENABLE_DOXYGEN`, target `vneio_doc_doxygen`).
- [CONTRIBUTING.md](CONTRIBUTING.md) — Contribution guidelines.
- [CODING_GUIDELINES.md](CODING_GUIDELINES.md) — Project conventions (aligned with other VertexNova libraries).

## Prebuilt binaries (GitHub Releases)

Each tagged release publishes install trees as `vneio-v{VERSION}-{platform}.tar.gz`.

| `platform` | Contents |
|------------|----------|
| `linux-gcc` | Shared library, headers, LICENSE, CHANGELOG |
| `macos` | Shared library, headers, LICENSE, CHANGELOG |
| `windows` | Shared library / DLL, headers, LICENSE, CHANGELOG |
| `web-emscripten` | Emscripten build (mesh disabled in CI/release) |
| `ios-static` | Static `.a`, headers, LICENSE, CHANGELOG (arm64) |

## Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Linux | Supported | GCC 10+, Clang 10+ |
| macOS | Supported | Xcode 12+, Apple Clang |
| Windows | Supported | MSVC 2019+, MinGW |
| iOS / visionOS | Supported | Via vnescene / vnemath toolchain |
| Android / Web | Experimental | Via vnescene / vnemath |

## Requirements

- **C++20**
- **CMake** 3.16+
- **Compiler**: GCC 10+, Clang 10+, MSVC 2019+
- **[Assimp](https://github.com/assimp/assimp)** (mesh component; submodule or system install)
- **[NrrdIO](https://teem.sourceforge.net/nrrd/lib.html)** (image/volume component; submodule or system install)
- **[stb_image](https://github.com/nothings/stb)** (image component; auto-fetched if absent)
- **Google Test** (tests; vendored via `deps/external/googletest`)
- **[vnecommon](https://github.com/vertexnova/vnecommon)** (optional; logging utilities)
- **[vnelogging](https://github.com/vertexnova/vnelogging)** (optional; diagnostics)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [CODING_GUIDELINES.md](CODING_GUIDELINES.md). PRs use [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md); releases and changelog are driven by [release-please](https://github.com/googleapis/release-please) (Conventional Commits).

## License

Apache License 2.0 — see [LICENSE](LICENSE) for details.

---

<p align="center">
  Part of the <a href="https://github.com/vertexnova">VertexNova</a> project
</p>
