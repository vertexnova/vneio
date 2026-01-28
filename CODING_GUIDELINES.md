# VneIo Coding Guidelines

VneIo follows the **VertexNova** coding style. Summary and tooling:

- **Naming and style** — See [.copilot/coding-guidelines.md](.copilot/coding-guidelines.md) for the short reference (tables and examples).
- **Formatting** — Enforced by [.clang-format](.clang-format).
- **Static analysis / naming** — Enforced by [.clang-tidy](.clang-tidy).
- **Editor defaults** — [.editorconfig](.editorconfig).

## VneIo-specific

- Public API: `include/vertexnova/io/` (mesh/, image/, vneio.h).
- Implementation: `src/vertexnova/io/`.
- Use namespaces `VNE::Mesh` and `VNE::Image`; keep struct/class and function names consistent with the guidelines above.

For the full VertexNova C++ guidelines (used across events, logging, math, and io), refer to the main VertexNova documentation or the same structure in sibling repos (e.g. vneevents, vnelogging).
