# External dependencies

- **assimp** – Git submodule (https://github.com/assimp/assimp), checked out at a stable tag (e.g. v5.4.3).
  - Init/update: `git submodule update --init external/assimp`
  - Use another tag: `cd external/assimp && git fetch origin tag <tag> && git checkout <tag>`

- **stb_image** – Copy of `vertexnova/3rd_party/stb_image` (stb-based image load/save with a CMake build).
  - VneIo uses this when `external/stb_image/CMakeLists.txt` exists; otherwise it falls back to FetchContent for the nothings/stb headers.

- **nrrdio** – Teem nrrdio library for robust NRRD volume loading (supports compressed formats, advanced encodings).
  - Clone: `git clone https://git.code.sf.net/p/teem/nrrdio/nrrdio.git external/nrrdio/nrrdio`
  - Or place under `external/nrrdio/` or `3rd_party/nrrdio/` with CMakeLists.txt
  - VneIo uses nrrdio when available (via `VNEIO_USE_NRRDIO=ON`, default ON), otherwise falls back to built-in raw NRRD parser.
