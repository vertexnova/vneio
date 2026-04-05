# 06 — Asset registry (`AssetIO`)

Registers multiple loaders with **`AssetIO`** and loads **image**, **volume**, and **mesh** assets through one API.

## What it covers

- `LoadRequest` URIs / paths
- `LoadResult<T>` success and error handling
- Registry dispatch vs direct loader usage

## Dependencies

Uses paths under **`testdata/`** for image, volume, and mesh sections (e.g. `textures/sample.png`, `volumes/small3d.nrrd`, `meshes/minimal.stl`). Missing files log a warning and the run continues where possible.

## Run

```bash
./build/bin/examples/06_AssetRegistry
```

See [`testdata/README.md`](../../testdata/README.md).
