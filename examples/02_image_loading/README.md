# 02 — Image loading

2D image **load → inspect → resize → save** using the `Image` type and helpers.

## What it covers

- Load from disk via `Image::loadFromFile`
- Dimensions, channels, pixel layout / GPU-upload readiness checks
- Resize and save paths (when test data is present)

## Dependencies

- **`testdata/textures/sample.png`** (or other `sample.*` under `textures/` — the example uses `sample.png`)

If the file is missing, the example **soft-skips** remaining checks and exits successfully; watch log output.

## Run

```bash
./build/bin/examples/02_ImageLoading
```

Run from the repo root or rely on CMake’s `VNEIO_TESTDATA_DIR` (see [`testdata/README.md`](../../testdata/README.md)).
