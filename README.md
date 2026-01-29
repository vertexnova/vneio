# VertexNova I/O (vneio)

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

Mesh and Image library extracted from VertexNova core, structured like **vneevents** / **vnelogging** / **vnemath**. It provides:

- **Mesh** – Load 3D meshes (Assimp), with vertex attributes, materials, submeshes, optional normalization and barycentrics.
- **Image** – Load/save images (stb_image), resize, flip, raw pixel access.

Namespaces: `VNE::Mesh`, `VNE::Image`. Includes live under `vertexnova/io/`.

## Requirements

- C++20
- CMake 3.16+
- **Mesh:** [Assimp](https://github.com/assimp/assimp) (optional; add under `3rd_party/assimp` or use `find_package(assimp)`)
- **Image:** [stb](https://github.com/nothings/stb) (fetched automatically if `3rd_party/stb_image` is not present)
- **Logging (optional):** [vnelogging](https://github.com/vertexnova/vnelogging) and [vnecommon](https://github.com/vertexnova/vnecommon) under `libs/` for mesh load logging

## Build

Clone with submodules (for vnecmake, vnecommon, vnelogging, external/assimp, external/googletest):

```bash
git clone --recursive https://github.com/vertexnova/vneio.git
cd vneio
# Or, if already cloned: git submodule update --init --recursive
```

**Test data (for `VNEIO_BUILD_TESTS=ON`):** Minimal assets live in `testdata/` (e.g. `meshes/minimal.stl`, `textures/sample.png`). No LFS or extra submodules required. If you had the old `testdata/vneresources` submodule, run `git submodule deinit testdata/vneresources` and remove the folder if desired.

Then build:

```bash
mkdir build && cd build
cmake .. -DVNEIO_BUILD_MESH=ON -DVNEIO_BUILD_IMAGE=ON
cmake --build .

# With tests (requires external/googletest; testdata/ is in repo):
# cmake .. -DVNEIO_BUILD_MESH=ON -DVNEIO_BUILD_IMAGE=ON -DVNEIO_BUILD_TESTS=ON
# cmake --build . && ctest --output-on-failure
```

- **VNEIO_BUILD_MESH** – build mesh component (default ON; needs Assimp).
- **VNEIO_BUILD_IMAGE** – build image component (default ON; stb fetched if needed).
- **VNEIO_USE_LOGGING** – use vnelogging when available (default ON). If OFF or libs not present, mesh uses no-op logging.
- **VNEIO_BUILD_TESTS** – build tests (default OFF). Enable with `-DVNEIO_BUILD_TESTS=ON`.
- **VNEIO_BUILD_EXAMPLES** – build examples (default OFF). Enable with `-DVNEIO_BUILD_EXAMPLES=ON`.
- **ENABLE_COVERAGE** – enable code coverage (default OFF). Use with Debug + GCC/Clang and lcov for reports.

To use a local Assimp or stb_image, place them under `3rd_party/assimp` and `3rd_party/stb_image` with their own `CMakeLists.txt` so that `add_subdirectory(3rd_party/...)` works.

## Usage

Link against `vne::io` (or `vne::io::mesh` / `vne::io::image`). Include:

```cpp
#include <vertexnova/io/vneio.h>
// or
#include <vertexnova/io/mesh/mesh.h>
#include <vertexnova/io/mesh/assimp_loader.h>
#include <vertexnova/io/image/image.h>
```

### Mesh

```cpp
#include <vertexnova/io/mesh/assimp_loader.h>

VNE::Mesh::AssimpLoader loader;
VNE::Mesh::Mesh mesh;
if (loader.loadFile("model.obj", mesh)) {
    // mesh.vertices, mesh.indices, mesh.parts, mesh.materials
}
```

### Image

```cpp
#include <vertexnova/io/image/image.h>

VNE::Image::Image img("texture.png");
if (!img.isEmpty()) {
    int w = img.getWidth(), h = img.getHeight();
    const uint8_t* data = img.getData();
}
```

## Layout (like vneevents/vnelogging)

- `include/vertexnova/io/` – public headers (mesh/, image/, vneio.h)
- `src/vertexnova/io/` – implementation (mesh/, image/)
- `cmake/vnecmake/` – shared CMake modules (submodule)
- `libs/vnecommon/`, `libs/vnelogging/` – submodules used when `VNEIO_USE_LOGGING` is ON
- `external/assimp`, `external/stb_image` – mesh/image dependencies (assimp is submodule, stb_image is a copy)
- `tests/`, `examples/` – optional

## License

Apache-2.0. Same as the original VertexNova core code.
