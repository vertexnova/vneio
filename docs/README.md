# VneIo documentation

This directory holds **design notes**, **diagram sources**, and the **Doxygen** template for API HTML. The public C++ API lives under `include/vertexnova/io/`; narrative architecture and CMake integration are described in the guides below.

## Architecture and usage

| Resource | Description |
|----------|-------------|
| [**vertexnova/io/vneio.md**](vertexnova/io/vneio.md) | End-to-end module design: layers, loaders, `AssetIO`, build options, integration patterns, and links to diagrams. Start here for “how vneio fits together.” |

## Diagrams (Draw.io)

Sources are **`.drawio`** files under [`vertexnova/io/diagrams/`](vertexnova/io/diagrams/). Export to PNG when you need checked-in previews referenced from Markdown (e.g. in `vneio.md`).

| File | Typical use |
|------|-------------|
| `context.drawio` | System / context view |
| `component.drawio` | Public headers vs `src/` layout |
| `class.drawio` | Core types and loader interfaces |
| `runtime.drawio` | Load pipeline and runtime flow |

## API reference (Doxygen)

| File | Role |
|------|------|
| [**doxyfile.in**](doxyfile.in) | Doxygen configuration template (`@CMAKE_PROJECT_NAME@`, `@PROJECT_SOURCE_DIR@`, `@CMAKE_BINARY_DIR@`, version, and `INPUT` paths are filled by CMake `configure_file`). |

HTML output directory is set to **`${CMAKE_BINARY_DIR}/docs`** in the template (Doxygen’s usual `html/` subfolder holds the site).

**Generating docs today**

1. Install [Doxygen](https://www.doxygen.nl/) (and [Graphviz](https://graphviz.org/) `dot` if you want call graphs; the template expects `@DOXYGEN_HAVE_DOT@` to be set when configuring the file).
2. From a configured build tree, generate a real `Doxyfile` from `docs/doxyfile.in` with the same `@ONLY` substitutions your project uses, then run `doxygen` with that file.

The repo includes a reusable helper in [`cmake/vnecmake/modules/Doxygen.cmake`](../cmake/vnecmake/modules/Doxygen.cmake): `enable_doxygen()` creates an **`doc_doxygen`** custom target when **`ENABLE_DOXYGEN=ON`** and Doxygen is found. The vneio top-level `CMakeLists.txt` does **not** call this yet; wiring it in is the straightforward way to get `cmake --build build --target doc_doxygen` working without hand-editing paths.

## See also

- Repository [README.md](../README.md) — overview, features, quick start, and links back to these docs.
- [CONTRIBUTING.md](../CONTRIBUTING.md) and [CODING_GUIDELINES.md](../CODING_GUIDELINES.md) — contribution and style expectations.
