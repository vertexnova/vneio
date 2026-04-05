# 07 — Load performance

**Synthetic** benchmarks for volume, image, and mesh **load throughput** (wall-clock). No checkout of large assets and **no `testdata/` directory required**.

## What it covers

- Volume loaders (NRRD, MHD/MHA) on generated grids
- Image (PNG) load timing
- Mesh OBJ load baseline vs post-processed path
- Optional `AssetIO` dispatch overhead section

Synthetic artifacts are written under **`VNEIO_TEST_OUTPUT_DIR`** (typically `CMAKE_BINARY_DIR/test_output`) via `tmpPath()` in [`examples/common/example_utils.h`](../common/example_utils.h) for the duration of the run.

## Run

```bash
./build/bin/examples/07_Performance
```

## Purpose

Compare loader and registry cost in a repeatable, self-contained way on your machine. Numbers are environment-dependent; use for **relative** comparisons after code changes, not as absolute SLAs.
