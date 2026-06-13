#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/export.h"

#include <string>

namespace vne {
namespace image {

/**
 * @brief Compression wrapper on disk.
 *
 * @note Gzip/tar are transport layers — not semantic volume formats (unlike NRRD/MHD).
 */
enum class VolumeCompression {
    eNone = 0,
    eGzip = 1,
};

/** @brief Archive wrapper on disk (USTAR tar of member files). */
enum class VolumeArchive {
    eNone = 0,
    eTar = 1,
};

/**
 * @brief Voxel memory layout inside the payload (after decompress / extract).
 *
 * @note Requires caller hints or heuristics — not encoded in tar/gzip themselves.
 */
enum class VolumeVoxelLayout {
    eUnknown = 0,
    eSliceStack,  //!< Numbered 2D slice files stacked into a 3D grid (Stanford tar datasets).
    eDenseGrid,   //!< Single contiguous WxHxD buffer (BrainWeb `.bin-gz`).
};

/** @brief Self-describing volume formats whose headers carry dims and pixel type. */
enum class VolumeSemanticFormat {
    eNone = 0,
    eNrrd,
    eMhd,
};

/**
 * @brief Parsed path classification: wrappers + inferred layout + semantic format.
 *
 * Examples:
 * - `CThead.tar`       → none + tar + slice_stack
 * - `CThead.tar.gz`    → gzip + tar + slice_stack
 * - `brain....bin-gz`  → gzip + none + dense_grid
 * - `volume.nrrd`      → semantic NRRD (wrappers unused)
 *
 * Bare `.gz` is intentionally @c eUnknown — it may wrap tar or a flat blob; use `.tar.gz` or
 * `.bin-gz` so the loader route is unambiguous.
 */
struct VolumePathDescriptor {
    VolumeCompression compression = VolumeCompression::eNone;
    VolumeArchive archive = VolumeArchive::eNone;
    VolumeVoxelLayout layout = VolumeVoxelLayout::eUnknown;
    VolumeSemanticFormat semantic = VolumeSemanticFormat::eNone;

    [[nodiscard]] bool hasSemanticFormat() const { return semantic != VolumeSemanticFormat::eNone; }

    /** @brief True when @ref VolumeLoader knows which loader to invoke. */
    [[nodiscard]] bool isRoutable() const;
};

/** @brief Classify @p path from extension only (does not read file bytes). */
[[nodiscard]] VNEIO_API VolumePathDescriptor parseVolumePath(const std::string& path);

/** @brief Short description for logs (e.g. "gzip + tar + slice-stack"). */
[[nodiscard]] VNEIO_API std::string describeVolumePath(const VolumePathDescriptor& desc);

/**
 * @brief High-level loader route derived from @ref parseVolumePath.
 *
 * @deprecated Prefer @ref VolumePathDescriptor; kept for dispatch switches.
 */
enum class VolumeLoadRoute {
    eUnknown = 0,
    eSliceStackArchive,  //!< `.tar` or `.tar.gz` / `.tgz` numbered slices.
    eDenseGridGzip,      //!< `.bin-gz` gzip-wrapped dense voxel blob.
    eNrrd,
    eMhd,
};

[[nodiscard]] VNEIO_API VolumeLoadRoute volumeLoadRouteFromPath(const std::string& path);

/** @brief Maps a routable descriptor to a loader route. */
[[nodiscard]] VNEIO_API VolumeLoadRoute volumeLoadRouteFromDescriptor(const VolumePathDescriptor& desc);

[[nodiscard]] VNEIO_API const char* volumeLoadRouteName(VolumeLoadRoute route);

// Backward-compatible aliases (tar/gzip are routes, not semantic volume formats).
using VolumeContainerFormat = VolumeLoadRoute;
[[nodiscard]] inline VolumeLoadRoute detectVolumeContainerFormat(const std::string& path) {
    return volumeLoadRouteFromPath(path);
}
[[nodiscard]] inline const char* volumeContainerFormatName(const VolumeLoadRoute route) {
    return volumeLoadRouteName(route);
}

}  // namespace image
}  // namespace vne
