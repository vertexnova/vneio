# Contributing to VneIo

Thank you for your interest in contributing. This document explains how to build, test, and format the project so your contributions fit the repository standards.

## Building

- **Prerequisites:** CMake 3.16+, C++20 compiler. See [README.md](README.md).
- **Dependencies:** Initialize submodules from the project root:
  ```bash
  git submodule update --init --recursive
  ```
- **Configure and build** (typical layout — adjust `shared`/`static` and `Release`/`Debug` as needed):
  ```bash
  cmake -B build/shared/Release -S . -DCMAKE_BUILD_TYPE=Release -DVNEIO_CI=ON -DVNEIO_BUILD_TESTS=ON -DVNEIO_LIB_TYPE=shared
  cmake --build build/shared/Release --parallel
  ```
- **Dev defaults:** `-DVNEIO_DEV=ON` enables tests (and examples when present) with convenient defaults.
- **Platform scripts:** See [scripts/README.md](scripts/README.md) for `build_linux.sh`, `build_macos.sh`, `build_web.sh`, `build_ios.sh`, and Windows helpers.

## Testing

- Run tests:
  ```bash
  ctest --test-dir build/shared/Release -C Release --output-on-failure
  ```
- Or use a script, e.g. `./scripts/build_macos.sh -a test -t Release -l shared`.

## Code style and formatting

- **Formatting** is enforced in CI. Before pushing, run:
  ```bash
  python3 scripts/clang_formatter.py all --dry-run
  ```
  (`--folder all` is equivalent.) Remove `--dry-run` to apply fixes. CI uses `clang-format-17` when available (see [.github/workflows/ci.yml](.github/workflows/ci.yml)).
- **Style and naming** are defined in [CODING_GUIDELINES.md](CODING_GUIDELINES.md). Static analysis uses [.clang-tidy](.clang-tidy).

## Pull requests

- Ensure the project builds and tests pass (locally or via CI).
- Keep changes focused. [CHANGELOG.md](CHANGELOG.md) is updated by [release-please](https://github.com/googleapis/release-please) from [Conventional Commits](https://www.conventionalcommits.org/)–style PR titles when releases are cut.
- If you add or change behavior, add or update tests as appropriate.

## Code review

- Reviews follow [.github/copilot-instructions.md](.github/copilot-instructions.md) and [CODING_GUIDELINES.md](CODING_GUIDELINES.md). Addressing feedback on correctness, API design, and test quality will speed up merging.
