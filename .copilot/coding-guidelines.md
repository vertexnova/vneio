# VertexNova Coding Guidelines (VneIo)

**Version:** 2.0
**Last Updated:** January 2026
**C++ Standard:** C++17 minimum, C++20 preferred

## Naming Conventions Summary

| Construct | Style | Example |
|-----------|-------|---------|
| Classes/Structs | PascalCase | `Buffer`, `ShaderCompiler` |
| Interface classes | `I` + PascalCase | `IRenderer`, `IBuffer` |
| Enums | PascalCase | `LogSink`, `ShaderStage` |
| Enum values | `e` + PascalCase + explicit value | `eNone = 0`, `eConsole = 1` |
| Type aliases | PascalCase | `EntityId`, `BufferHandle` |
| Functions/Methods | camelCase | `initialize()`, `createBuffer()` |
| Constants | `k` + PascalCase (in anonymous namespace) | `kMaxBufferSize` |
| Private/Protected members | snake_case + `_` | `buffer_size_`, `is_initialized_` |
| Public members | snake_case | `buffer_size`, `is_initialized` |
| Local variables | snake_case | `buffer_size`, `file_path` |
| Function parameters | snake_case | `buffer_size`, `usage` |
| Static members (private) | `s_` + snake_case + `_` | `s_instance_count_` |
| Static members (public) | `s_` + snake_case | `s_instance_count` |
| Global variables | `g_` + snake_case | `g_instance`, `g_config` |
| Booleans | `is_`, `has_`, `can_`, `should_` prefix | `is_ready_`, `has_alpha_` |
| Macros | ALL_CAPS with VNE_ prefix | `VNE_ASSERT`, `VNE_PLATFORM_WINDOWS` |
| Namespaces | lowercase | `vne`, `VNE::Mesh`, `VNE::Image` |
| File names | snake_case | `shader_compiler.h` |
| Header guards | `#pragma once` | `#pragma once` |

## VneIo-specific layout

- Public API lives under `include/vertexnova/io/` (mesh/, image/, vneio.h).
- Implementation under `src/vertexnova/io/`.
- Use namespaces `VNE::Mesh` and `VNE::Image`. Follow the same naming rules as above for types and functions in mesh/image code.

## Code Formatting

- **4 spaces** per indentation level (no tabs)
- **120 characters** maximum line length
- **LF** line endings for all platforms
- **`#pragma once`** for header guards
- **Attached braces** (opening brace on same line)
- **Left-aligned pointers**: `int* ptr;`

## Tool Enforcement

- **`.clang-format`** – automatic formatting
- **`.clang-tidy`** – static analysis and naming
- **`.editorconfig`** – editor settings

See the repository `CODING_GUIDELINES.md` (if present) or the root coding guidelines for full documentation.
