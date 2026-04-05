# Test data

Small, committed fixtures for **VneIo** unit tests (`TestVneIo`) and **examples** under `examples/`. Nothing here uses Git LFS or extra submodules.

## Layout

| Directory | Role |
|-----------|------|
| `textures/` | Raster images (several formats) for `StbImageLoader` and image pipeline demos. |
| `volumes/` | NRRD and MetaImage (MHD + RAW, MHA) volumes for `NrrdLoader` / `MhdLoader` and volume tests. |
| `meshes/` | Tiny meshes (STL, PLY, USD) for `AssimpLoader` and mesh examples. |

## Files

### `textures/`

Same test pattern exported to multiple formats so loaders can be exercised without large assets:

- `sample.png`, `sample.jpg`, `sample.jpeg`, `sample.bmp`, `sample.gif`, `sample.tga`

Examples and `tests/image/image_test.cpp` typically use `textures/sample.png`.

### `volumes/`

- **`small3d.nrrd`** — Attached NRRD, 4×4×4 `uint8`, voxel values 0…63 (ramp). Used by volume tests and examples.
- **`small3d.mhd`** + **`small3d.raw`** — Detached MetaImage with the **same** 4×4×4 uchar volume as `small3d.nrrd`.
- **`small3d.mha`** — Single-file MetaImage variant of the same volume.
- **`an-hist.nrrd`** — 1D NRRD (256-bin histogram, ASCII encoding); used for loader smoke tests.
- **`fool.nrrd`** — 2D NRRD; tests expect depth padded to 1 when loaded as a volume.

### `meshes/`

- **`minimal.stl`**, **`minimal.ply`** — Minimal triangle geometry for mesh load / round-trip examples.
- **`minimal.usd`** — Tiny USDA scene (single triangle) for USD coverage via Assimp.

## How paths are resolved

- **CMake builds** set `VNEIO_TESTDATA_DIR` to an **absolute** path: `<source-root>/testdata` (see root `CMakeLists.txt` and `tests/CMakeLists.txt`, `examples/CMakeLists.txt`).
- **Tests** use `vne::io::utils::getTestdataPath("relative/path")` so paths work regardless of the test binary location.
- **Examples** concatenate `VNEIO_TESTDATA_DIR` with paths such as `textures/sample.png` or `meshes/minimal.stl`.

If you run a binary by hand without those definitions, run it from the **repository root** with a relative `testdata/` tree, or define `VNEIO_TESTDATA_DIR` yourself to point at this directory.

## Adding data

Prefer **tiny** files (low resolution, few vertices) so the repo stays fast to clone and CI stays quick. Keep names stable if tests reference them by path.
