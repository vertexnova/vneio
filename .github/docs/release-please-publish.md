# Release workflow: prebuilt artifacts (`release-please.yml`)

This document describes how the **Release / publish** job names tarballs, what each install tree contains, and how to debug common CI failures. The workflow file keeps a short copyright header only; details live here.

## Artifact naming

Published files:

`vneio-v{VERSION}-{matrix.platform}-{ARTIFACT_DETAIL}.tar.gz`

`matrix.platform` is one of: `linux-gcc`, `macos`, `windows`, `web-emscripten`, `ios-static`, `android`.

`ARTIFACT_DETAIL` is computed in the **Compute artifact detail suffix** step.

| Platform         | `ARTIFACT_DETAIL` rule |
|------------------|------------------------|
| **linux-gcc**    | `ubuntu{VERSION_ID (no dots)}-gcc{gcc version (no dots)}-{uname -m}`; if GCC is missing, `ubuntu{VERSION_ID}-{arch}` with a notice. |
| **macos**        | `macos{productVersion (no dots)}-xcode{xcodebuild version (no dots)}-{uname -m}` when Xcode parses; if the OS version is missing, `macos-xcode{…}-{arch}`; if Xcode does not parse, `macos{…}-{arch}` or `macos-{arch}` with a notice. The literal string `unknown` is never used in filenames. |
| **windows**      | `vs2022-x64` (fixed). |
| **web-emscripten** | `emcc{emcc -dumpversion}-{uname -m}`; fails the job if the version is missing or `unknown`. |
| **ios-static**   | `xcode{xcodebuild version (no dots)}-arm64`. The job **fails early** if Xcode is missing or the version cannot be read (no `xcodeunknown-arm64` or similar). |
| **android**      | `android{API}-ndk{NDK Pkg.Revision (no dots)}-arm64v8a`, or `android{API}-arm64v8a` if NDK revision cannot be read. |

Examples (runner-dependent):

- `vneio-v1.2.3-linux-gcc-ubuntu2204-gcc1320-x86_64.tar.gz`
- `vneio-v1.2.3-macos-macos1421-xcode260-arm64.tar.gz`
- `vneio-v1.2.3-ios-static-xcode260-arm64.tar.gz`
- `vneio-v1.2.3-android-android24-ndk27212479018-arm64v8a.tar.gz`

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

If `xcodebuild -version` is missing or unusable (wrong `DEVELOPER_DIR`, Xcode not selected, broken image), the **iOS** publish job fails in **Verify Xcode (iOS release)** with a clear error instead of producing an invalid tarball name.

macOS desktop jobs avoid embedding the literal `unknown` in `ARTIFACT_DETAIL` when OS or Xcode metadata is missing.

### Getting more compiler output

- iOS **Configure**: `CMAKE_VERBOSE_MAKEFILE=ON`
- iOS **Build**: `VERBOSE=1` and `cmake --build … --verbose`
- iOS **Install**: `cmake --install … --verbose`

The same verbosity applies to the **Build / iOS** job in `ci.yml` for parity.
