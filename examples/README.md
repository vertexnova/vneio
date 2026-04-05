# VneIo — Examples

Headless (no window, no GPU) programs that exercise **VertexNova I/O** through the public API: images, volumes, meshes, and the unified `AssetIO` registry. Each example uses a thin `main.cpp` plus `NN_example.h` / `NN_example.cpp` (matching the folder number), with runnable logic in `namespace vne::io::examples`, and builds as a **standalone executable**.

Small fixtures live under [`testdata/`](../testdata/); see [`testdata/README.md`](../testdata/README.md) for layout and path rules. Example **07** uses only synthetic files under `/tmp` and does not need `testdata/`.

## Building

```bash
# Dev preset (typical local workflow: tests + examples)
cmake -B build -DVNEIO_DEV=ON
cmake --build build

# Examples only (when not using VNEIO_DEV)
cmake -B build -DVNEIO_BUILD_EXAMPLES=ON
cmake --build build
```

Examples are **disabled** for `VNEIO_CI=ON`, `ENABLE_COVERAGE=ON`, and for iOS/Web targets (see root `CMakeLists.txt` and `examples/CMakeLists.txt`).

With **shared** `libvneio_*`, VertexNova Logging headers are picked up when `deps/internal/vnelogging` is present; the example links `vne::logging` so `LoggingGuard` can configure real loggers. Otherwise examples fall back to console `VNE_LOG_*` stubs.

Executables are written to **`build/bin/examples/`** (exact path depends on your build directory name).

## Running

Run from the repository root (or ensure `VNEIO_TESTDATA_DIR` points at `testdata/` — CMake sets this to an absolute path for configured builds):

```bash
./build/bin/examples/01_LibraryInfo
./build/bin/examples/02_ImageLoading
./build/bin/examples/03_VolumeLoading
# … etc.
```

If `testdata/` is missing or paths are wrong, several examples **soft-skip** file-based sections and still exit 0; check the console output.

---

## Examples

| # | Directory | Executable | What it covers |
|---|-----------|------------|----------------|
| 01 | [`01_library_info`](01_library_info) | `01_LibraryInfo` | Supported formats, pixel types, loader registries, `Status` — linkage smoke test; **no file I/O** |
| 02 | [`02_image_loading`](02_image_loading) | `02_ImageLoading` | `Image` load / inspect / resize / save; `testdata/textures/sample.png` |
| 03 | [`03_volume_loading`](03_volume_loading) | `03_VolumeLoading` | `NrrdLoader` on `testdata` NRRDs; synthetic MHD write/load; `MhdLoader` + `LoadRequest` |
| 04 | [`04_volume_export`](04_volume_export) | `04_VolumeExport` | `VolumeExporter` round-trip: attached NRRD, detached NRRD (`.nhdr` + `.raw`), MHD + RAW, inline MHA |
| 05 | [`05_mesh_loading`](05_mesh_loading) | `05_MeshLoading` | `AssimpLoader`, `Mesh` inspection, AABB, OBJ export + reload; `testdata/meshes/minimal.stl` |
| 06 | [`06_asset_registry`](06_asset_registry) | `06_AssetRegistry` | `AssetIO` registry, `LoadRequest`, `LoadResult<T>` for image / volume / mesh |
| 07 | [`07_performance`](07_performance) | `07_Performance` | Load-time benchmarks with **synthetic** assets under `/tmp` — **no `testdata` required** |

Each numbered folder has its own **README.md** with a short description of scope and dependencies.

---

## Common helpers (`common/`)

| File | Purpose |
|------|---------|
| [`logging_guard.h`](common/logging_guard.h) | RAII logging: configures VertexNova logging when `VNEIO_EXAMPLES_HAS_LOGGING` is set (shared lib + vnelogging present), otherwise console stubs compatible with `VNE_LOG_*` |
| [`example_utils.h`](common/example_utils.h) | Shared helpers (`printSection`, checks, timing helpers, etc.) for headless demos |
