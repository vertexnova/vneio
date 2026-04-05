/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * @file 04_example.cpp
 * @brief Example 04: Volume export round-trip — synthesise a volume and prove it survives all four export / reload
 * paths.
 *
 * Paths: .nrrd (attached); .nhdr + .raw (detached NRRD); .mhd + .raw; .mha (inline MHD).
 * Each variant checks geometry metadata and full voxel payload equality to guard against
 * silent truncation or byte-order bugs.
 */

#include "04_example.h"
#include "example_utils.h"

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_exporter.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

namespace vne::io::examples {

static constexpr float kEps = 5e-4f;

static bool nearEq(float a, float b) {
    return std::fabs(a - b) < kEps;
}

// Verify that `loaded` matches `src` geometry and raw voxel bytes.
static bool verifyRoundTrip(const vne::image::Volume& src, const vne::image::Volume& loaded, const char* label) {
    bool ok = true;
    ok &= check(loaded.dims[0] == src.dims[0] && loaded.dims[1] == src.dims[1] && loaded.dims[2] == src.dims[2],
                (std::string(label) + ": dims match").c_str());
    ok &= check(nearEq(loaded.spacing[0], src.spacing[0]) && nearEq(loaded.spacing[1], src.spacing[1])
                    && nearEq(loaded.spacing[2], src.spacing[2]),
                (std::string(label) + ": spacing match").c_str());
    ok &= check(nearEq(loaded.origin[0], src.origin[0]) && nearEq(loaded.origin[1], src.origin[1])
                    && nearEq(loaded.origin[2], src.origin[2]),
                (std::string(label) + ": origin match").c_str());
    for (int i = 0; i < 9; ++i) {
        ok &= check(nearEq(loaded.direction[i], src.direction[i]),
                    (std::string(label) + ": direction[" + std::to_string(i) + "] match").c_str());
    }
    ok &= check(loaded.pixel_type == src.pixel_type, (std::string(label) + ": pixel_type match").c_str());
    ok &= check(loaded.byteCount() == src.byteCount(), (std::string(label) + ": byteCount match").c_str());
    ok &= check(loaded.data == src.data, (std::string(label) + ": voxel payload match").c_str());
    return ok;
}

int runVolumeExportExample() {
    LoggingGuard logging_guard;

    // ── Build reference volume ────────────────────────────────────────────────
    printSection("Build synthetic 4x4x4 uint16 volume");
    vne::image::Volume src;
    src.dims[0] = src.dims[1] = src.dims[2] = 4;
    src.spacing[0] = 0.75f;
    src.spacing[1] = 1.25f;
    src.spacing[2] = 2.0f;
    src.origin[0] = 10.0f;
    src.origin[1] = -5.0f;
    src.origin[2] = 0.5f;
    // Non-identity direction (slight rotation in XY)
    src.direction[0] = 0.9998477f;
    src.direction[1] = -0.0174524f;
    src.direction[2] = 0.0f;
    src.direction[3] = 0.0174524f;
    src.direction[4] = 0.9998477f;
    src.direction[5] = 0.0f;
    src.direction[6] = 0.0f;
    src.direction[7] = 0.0f;
    src.direction[8] = 1.0f;
    src.pixel_type = vne::image::VolumePixelType::eUint16;
    src.components = 1;
    src.data.resize(src.byteCount());
    for (size_t i = 0; i < src.voxelCount(); ++i) {
        const uint16_t v = static_cast<uint16_t>(i * 100 + 1);
        std::memcpy(src.data.data() + i * sizeof(uint16_t), &v, sizeof(uint16_t));
    }
    printVolumeInfo(src, "source");
    if (!check(src.isMetadataValid(), "source isMetadataValid()==true")) {
        return 1;
    }

    // ── Variant 1: .nrrd attached ─────────────────────────────────────────────
    printSection("Variant 1: NRRD attached (.nrrd)");
    {
        const std::string path = tmpPath("vneio_ex04_v1.nrrd");
        std::string err;
        vne::image::NrrdExportOptions opts;
        if (!check(vne::image::exportNrrd(path, src, opts, &err), ("exportNrrd: " + path).c_str())) {
            VNE_LOG_ERROR << "error: " << err;
            return 1;
        }
        vne::image::NrrdLoader loader;
        vne::image::Volume loaded;
        if (!check(loader.load(path, loaded), ("reload: " + path).c_str())) {
            VNE_LOG_ERROR << "reload error: " << loader.getLastError();
            return 1;
        }
        if (!verifyRoundTrip(src, loaded, "nrrd-attached")) {
            return 1;
        }
        printVolumeInfo(loaded, "loaded (nrrd attached)");
    }

    // ── Variant 2: .nhdr + .raw detached ──────────────────────────────────────
    printSection("Variant 2: NRRD detached (.nhdr + .raw)");
    {
        const std::string path = tmpPath("vneio_ex04_v2.nhdr");
        std::string err;
        vne::image::NrrdExportOptions opts;
        opts.detached_data = true;
        if (!check(vne::image::exportNrrd(path, src, opts, &err), ("exportNrrd detached: " + path).c_str())) {
            VNE_LOG_ERROR << "error: " << err;
            return 1;
        }
        vne::image::NrrdLoader loader;
        vne::image::Volume loaded;
        if (!check(loader.load(path, loaded), ("reload: " + path).c_str())) {
            VNE_LOG_ERROR << "reload error: " << loader.getLastError();
            return 1;
        }
        if (!verifyRoundTrip(src, loaded, "nrrd-detached")) {
            return 1;
        }
        printVolumeInfo(loaded, "loaded (nrrd detached)");
    }

    // ── Variant 3: .mhd + .raw ────────────────────────────────────────────────
    printSection("Variant 3: MHD + .raw");
    {
        const std::string path = tmpPath("vneio_ex04_v3.mhd");
        std::string err;
        vne::image::MhdExportOptions opts;  // inline_data=false by default
        if (!check(vne::image::exportMhd(path, src, opts, &err), ("exportMhd: " + path).c_str())) {
            VNE_LOG_ERROR << "error: " << err;
            return 1;
        }
        vne::image::MhdLoader loader;
        vne::image::Volume loaded;
        if (!check(loader.load(path, loaded), ("reload: " + path).c_str())) {
            VNE_LOG_ERROR << "reload error: " << loader.getLastError();
            return 1;
        }
        if (!verifyRoundTrip(src, loaded, "mhd-separate")) {
            return 1;
        }
        printVolumeInfo(loaded, "loaded (mhd + raw)");
    }

    // ── Variant 4: .mha inline ────────────────────────────────────────────────
    printSection("Variant 4: MHA inline (.mha)");
    {
        const std::string path = tmpPath("vneio_ex04_v4.mha");
        std::string err;
        vne::image::MhdExportOptions opts;
        opts.inline_data = true;
        if (!check(vne::image::exportMhd(path, src, opts, &err), ("exportMhd inline: " + path).c_str())) {
            VNE_LOG_ERROR << "error: " << err;
            return 1;
        }
        vne::image::MhdLoader loader;
        vne::image::Volume loaded;
        if (!check(loader.load(path, loaded), ("reload: " + path).c_str())) {
            VNE_LOG_ERROR << "reload error: " << loader.getLastError();
            return 1;
        }
        if (!verifyRoundTrip(src, loaded, "mha-inline")) {
            return 1;
        }
        printVolumeInfo(loaded, "loaded (mha inline)");
    }

    VNE_LOG_INFO << "04_volume_export: done — all 4 round-trip variants passed.";
    return 0;
}

}  // namespace vne::io::examples
