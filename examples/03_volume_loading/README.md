# 03 — Volume loading

Loads **3D volumes** with `NrrdLoader` and `MhdLoader`, validates metadata, and inspects voxel data.

## What it covers

- **NRRD** from `testdata`: `small3d.nrrd`, `an-hist.nrrd`; metadata and voxel checks; `LoadRequest` path for `NrrdLoader`
- **MHD**: writes a **synthetic** volume to a temp `.mhd` + `.raw`, loads it back with `MhdLoader`, then exercises `loadVolume(LoadRequest)`

## Dependencies

**`testdata/volumes/small3d.nrrd`** and **`an-hist.nrrd`** for NRRD sections (skipped with a log line if missing). MHD sections use the system temp directory. See [`testdata/README.md`](../../testdata/README.md).

## Run

```bash
./build/bin/examples/03_VolumeLoading
```
