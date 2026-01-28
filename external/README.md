# External dependencies

- **assimp** – Git submodule (https://github.com/assimp/assimp), checked out at a stable tag (e.g. v5.4.3).  
  - Init/update: `git submodule update --init external/assimp`  
  - Use another tag: `cd external/assimp && git fetch origin tag <tag> && git checkout <tag>`

- **stb_image** – Copy of `vertexnova/3rd_party/stb_image` (stb-based image load/save with a CMake build).  
  - VneIo uses this when `external/stb_image/CMakeLists.txt` exists; otherwise it falls back to FetchContent for the nothings/stb headers.
