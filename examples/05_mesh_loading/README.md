# 05 — Mesh loading

Loads a mesh with **Assimp**, inspects geometry, and exercises **export + reload** (e.g. OBJ).

## What it covers

- `AssimpLoader` and `Mesh` — vertices, indices, attributes
- Axis-aligned bounds
- Optional round-trip through `MeshExporter` and reload

## Dependencies

- **`testdata/meshes/minimal.stl`** (primary path in the example)

If missing, the example **soft-skips** and exits 0.

## Run

```bash
./build/bin/examples/05_MeshLoading
```

See [`testdata/README.md`](../../testdata/README.md).
