# 07 — Load performance

**Synthetic** benchmarks for volume, image, and mesh **load throughput** (wall-clock). No checkout of large assets and **no `testdata/` directory required**.

## What it covers

- Volume loaders (NRRD, MHD/MHA) on generated grids
- Image (PNG) load timing
- Mesh OBJ load baseline vs post-processed path
- Optional `AssetIO` dispatch overhead section

Assets are written under **`/tmp`** (or the platform temp directory) for the duration of the run.

## Run

```bash
./build/bin/examples/07_Performance
```

## Purpose

Compare loader and registry cost in a repeatable, self-contained way on your machine. Numbers are environment-dependent; use for **relative** comparisons after code changes, not as absolute SLAs.
