# Release workflow: prebuilt artifacts (`release-please.yml`)

This document describes how the **Release / publish** job names tarballs, what each install tree contains, and how to debug common CI failures. The workflow file keeps a short copyright header only; details live here.

## Artifact naming

Published files:

`vneio-v{VERSION}-{matrix.platform}-{ARTIFACT_DETAIL}.tar.gz`

`matrix.platform` is one of: `linux-gcc`, `macos`, `windows`, `web-emscripten`, `ios-static`, `android`.

`ARTIFACT_DETAIL` is computed in the **Compute artifact detail suffix** step.

**Normalization:** Any field that is empty or the literal **`unknown`** (case-insensitive) is **left out** of the suffix — the workflow does **not** embed the token `unknown` in filenames. That includes **`VERSION_ID`**, **`uname -m`**, **GCC version**, **NDK revision**, and **macOS / Xcode** segments where applicable. If **`uname -m`** is missing, the trailing `-{arch}` piece is omitted (with a notice). If both **GCC** and **`VERSION_ID`** are unusable on Linux, the fallback prefix is `linux-gcc` before any arch suffix.

| Platform         | `ARTIFACT_DETAIL` rule |
|------------------|------------------------|
| **linux-gcc**    | Typically `ubuntu{VERSION_ID}-gcc{gcc -dumpfullversion}-{uname -m}` with dots preserved on Ubuntu/GCC strings. If **`VERSION_ID`** is missing or `unknown`, the `ubuntu{…}` segment is dropped (`gcc{…}-{arch}` or `linux-gcc-{arch}` / `linux-gcc` if needed). If GCC is missing, `ubuntu{VERSION_ID}-{arch}` or `ubuntu{VERSION_ID}`; notices as above. |
| **macos**        | Typically `macos{productVersion}-xcode{xcodeVersion}-{uname -m}` with dots preserved. If **product version** is missing or `unknown`, that segment is omitted (e.g. `macos-xcode{…}-{arch}`). If Xcode does not parse, the `xcode{…}` segment is omitted. Trailing `{arch}` follows the same rules as other platforms. |
| **windows**      | `vs2022-x64` (fixed). |
| **web-emscripten** | `emcc{emcc -dumpversion}-{uname -m}` when both parse; the job errors if the Emscripten version is missing or `unknown`. |
| **ios-static**   | `xcode{xcodebuild version}-arm64` with **dots preserved** (e.g. **26.0**). The Xcode version must be readable from `xcodebuild -version`; there is **no** generic fallback suffix (e.g. `ios-arm64`) if parsing fails. |
| **android**      | `android{API}-ndk{NDK Pkg.Revision}-arm64v8a` with **NDK revision dots preserved**, or `android{API}-arm64v8a` if NDK revision cannot be read. **{API}** is the numeric suffix of **`matrix.android_platform`** (e.g. `android-24` → `24`), the same string passed to **`-DANDROID_PLATFORM`** in **Configure CMake (Android)**. |

Examples (runner-dependent; version segments keep **dots** as reported by the OS/tooling):

- `vneio-v1.2.3-linux-gcc-ubuntu22.04-gcc13.2.0-x86_64.tar.gz`
- `vneio-v1.2.3-macos-macos14.2.1-xcode26.0-arm64.tar.gz`
- `vneio-v1.2.3-macos-macos14.2-xcode26.0-arm64.tar.gz` (two-part macOS product version)
- `vneio-v1.2.3-macos-xcode26.0-arm64.tar.gz` (when `sw_vers` did not yield a usable product version)
- `vneio-v1.2.3-ios-static-xcode26.0-arm64.tar.gz`
- `vneio-v1.2.3-android-android24-ndk27.2.12479018-arm64v8a.tar.gz`

## Install tree layout

After `cmake --install` into the staging prefix:

- `include/`, `lib/` (and `bin/` on Windows)
- `LICENSE`, `CHANGELOG.md` when present in the repo root
- Web: `.wasm` / `.js` (and related) under the prefix
- iOS: static libraries (`.a`) under the prefix (search is recursive)
- `VNEIO_SANITIZE_INSTALL=ON` removes embedded **vnelogging** / **vnecommon** headers and matching vendored libs from `lib/`; it must **not** remove `libvneio_*.a` / consumer-facing archives.

## Troubleshooting

### iOS: `expected at least one .a under install/`

This means the install step did not place any `.a` files under `install/`, or the build never produced installable static archives.

1. **Confirm the build succeeded** — open the **Build** log. The workflow uses `VERBOSE=1`, `cmake --build … --verbose`, and (for configure) `CMAKE_VERBOSE_MAKEFILE=ON` on iOS so compiler and link lines surface in the log.
2. **Inspect the staging install** — the **Install to staging directory** step prints groups listing `.a` files in the build tree and under `install/`.
3. **Check CMake install rules** — `CMakeLists.txt` installs `vneio_*` static targets to `CMAKE_INSTALL_LIBDIR`. If configure skipped targets (e.g. optional components), there may be nothing to install.
4. **Compare with local** — from a Mac: configure iOS like `ci.yml` / `release-please.yml`, then `cmake --build … --config Release` and `cmake --install … --config Release --prefix /tmp/vneio-install`, then `find /tmp/vneio-install -name '*.a'`.

### Xcode / artifact suffix errors

For **ios-static**, `release-please.yml` evaluates **`xcodebuild -version`** in several steps (not just one):

1. **Verify Xcode (iOS release)** — runs `xcodebuild -version` and errors if it fails or the Xcode version line cannot be read.
2. **Configure CMake (iOS)** — runs `xcodebuild -version` again (under `set -x`) before invoking CMake.
3. **Compute artifact detail suffix** — runs `xcodebuild -version` once more to build `xcode{version}-arm64`; errors if parsing still fails.

If Xcode is missing, wrong, or `DEVELOPER_DIR` is broken, open the failed publish job log, **search for `xcodebuild -version`**, and read the **`::error::`** line in the step that actually stopped (often the first of the three that runs). The table still applies: there is no ambiguous **`ios-arm64`-only** suffix when the version cannot be read.

### Getting more compiler output

For **ios-static** / **Build / iOS**, both **`release-please.yml`** and **`ci.yml`** pass **`-DCMAKE_VERBOSE_MAKEFILE=ON`** on the **`cmake -B … -S …`** line in the **Configure CMake (iOS)** step.

- iOS **Build**: **`VERBOSE=1`** and **`cmake --build … --config Release --parallel --verbose`** (this is usually where full compile/link lines show up with **`-G Xcode`**).
- iOS **Install** (release only): **`cmake --install … --verbose`**.

`ci.yml` **Build / iOS** mirrors the same configure flag and build verbosity as the release publish job.
