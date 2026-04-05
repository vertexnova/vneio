# 01 — Library info

Verifies linkage and walks the **public I/O surface** without reading files from disk.

## What it covers

- **Image** — `StbImageLoader` supported extensions; `Image` pixel type helpers
- **Volume** — `NrrdLoader` / `MhdLoader` registration hints; `Volume` / `VolumePixelType`
- **Mesh** — `AssimpLoader` supported extensions
- **Status** — success / error codes used across loaders

## Purpose

Use as a **smoke test** after building: if it runs and sections print expected loader metadata, the library is wired correctly. Good quick reference for which formats the build exposes.

## Run

```bash
./build/bin/examples/01_LibraryInfo
```

No `testdata/` required.
