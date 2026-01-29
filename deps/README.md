# Deps

Dependencies used by VneIo.

| Directory   | Purpose |
|------------|---------|
| `internal/` | VertexNova internal submodules (vnecommon, vnelogging). Used when `VNEIO_USE_LOGGING` is ON. |
| `external/` | External deps: assimp, googletest, stb_image, nrrdio (submodules/copy). |

Clone with submodules: `git submodule update --init --recursive` (or `git clone --recursive`).
