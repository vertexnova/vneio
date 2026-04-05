# 04 — Volume export (round-trip)

Builds a small synthetic volume and proves it survives **export and reload** across multiple on-disk representations.

## What it covers

- **Attached** NRRD (`.nrrd`)
- **Detached** NRRD (`.nhdr` + `.raw`)
- **MetaImage** detached (`.mhd` + `.raw`) and **inline** (`.mha`)
- Byte-for-byte / metadata equality checks after each round-trip

## Dependencies

Builds a **synthetic** reference volume in memory, writes exports under the **system temp directory**, and reloads with `NrrdLoader` / `MhdLoader`. **No `testdata/` required.**

## Run

```bash
./build/bin/examples/04_VolumeExport
```
