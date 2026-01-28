# VneIo

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

Clone with submodules (for vnecmake, vnecommon, vnelogging, and external/assimp):

```bash
git clone --recursive https://github.com/vertexnova/vneio.git
cd vneio
# Or, if already cloned: git submodule update --init --recursive
```

Then build:

```bash
mkdir build && cd build
cmake .. -DVNEIO_BUILD_MESH=ON -DVNEIO_BUILD_IMAGE=ON
cmake --build .
```

- **VNEIO_BUILD_MESH** – build mesh component (default ON; needs Assimp).
- **VNEIO_BUILD_IMAGE** – build image component (default ON; stb fetched if needed).
- **VNEIO_USE_LOGGING** – use vnelogging when available (default ON). If OFF or libs not present, mesh uses no-op logging.
- **BUILD_TESTS** / **BUILD_EXAMPLES** – off by default.

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
