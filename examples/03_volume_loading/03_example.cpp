/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 03: 3D volume load — NRRD loader, MHD loader, metadata
 * validation, voxel min/max, GPU-upload readiness checks.
 * ----------------------------------------------------------------------
 */

#include "03_example.h"
#include "example_utils.h"

#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_exporter.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

#ifndef VNEIO_TESTDATA_DIR
#define VNEIO_TESTDATA_DIR "testdata"
#endif

namespace vne::io::examples {

// Helper: compute min/max over a uint8 volume buffer
static void voxelMinMax(const vne::image::Volume& v, uint8_t& out_min, uint8_t& out_max) {
    const auto* p = v.dataAs<uint8_t>();
    out_min = 255;
    out_max = 0;
    for (size_t i = 0; i < v.voxelCount(); ++i) {
        if (p[i] < out_min) {
            out_min = p[i];
        }
        if (p[i] > out_max) {
            out_max = p[i];
        }
    }
}

// Helper: compute min/max over a uint16 volume buffer
static void voxelMinMaxU16(const vne::image::Volume& v, uint16_t& out_min, uint16_t& out_max) {
    const auto* p = v.dataAs<uint16_t>();
    size_t n = v.voxelCount();
    out_min = 65535;
    out_max = 0;
    for (size_t i = 0; i < n; ++i) {
        if (p[i] < out_min) {
            out_min = p[i];
        }
        if (p[i] > out_max) {
            out_max = p[i];
        }
    }
}

int runVolumeLoadingExample() {
    LoggingGuard logging_guard;

    // ── NRRD loader: small3d.nrrd ─────────────────────────────────────────────
    printSection("Load small3d.nrrd via NrrdLoader");
    {
        const std::string path = std::string(VNEIO_TESTDATA_DIR) + "/volumes/small3d.nrrd";
        vne::image::NrrdLoader loader;
        vne::image::Volume vol;
        bool ok = loader.load(path, vol);
        if (!ok) {
            VNE_LOG_WARN << "small3d.nrrd not found: " << loader.getLastError() << " — skipping NRRD tests";
        } else {
            printVolumeInfo(vol, "small3d.nrrd");
            if (!check(!vol.isEmpty(), "volume is not empty")) {
                return 1;
            }
            if (!check(vol.isMetadataValid(), "isMetadataValid()==true")) {
                return 1;
            }
            if (!check(vol.hasScalarVoxels(), "hasScalarVoxels()==true")) {
                return 1;
            }
            if (!check(vol.dims[0] > 0 && vol.dims[1] > 0 && vol.dims[2] > 0, "all dims > 0")) {
                return 1;
            }
            if (!check(vol.hasExactBufferSize(), "hasExactBufferSize()==true")) {
                return 1;
            }

            if (vol.pixel_type == vne::image::VolumePixelType::eUint8) {
                uint8_t mn, mx;
                voxelMinMax(vol, mn, mx);
                VNE_LOG_INFO << "  voxel min=" << static_cast<int>(mn) << "  max=" << static_cast<int>(mx);
            } else if (vol.pixel_type == vne::image::VolumePixelType::eUint16) {
                uint16_t mn, mx;
                voxelMinMaxU16(vol, mn, mx);
                VNE_LOG_INFO << "  voxel min=" << mn << "  max=" << mx;
            }
        }
    }

    // ── NRRD loader: an-hist.nrrd (if present) ────────────────────────────────
    printSection("Load an-hist.nrrd via NrrdLoader (optional)");
    {
        const std::string path = std::string(VNEIO_TESTDATA_DIR) + "/volumes/an-hist.nrrd";
        std::ifstream probe(path);
        if (!probe.good()) {
            VNE_LOG_INFO << "  an-hist.nrrd not present — skipping";
        } else {
            probe.close();
            vne::image::NrrdLoader loader;
            vne::image::Volume vol;
            bool ok = loader.load(path, vol);
            if (!check(ok, ("load an-hist.nrrd: " + path).c_str())) {
                return 1;
            }
            printVolumeInfo(vol, "an-hist.nrrd");
            if (!check(!vol.isEmpty(), "volume is not empty")) {
                return 1;
            }
            if (!check(vol.isMetadataValid(), "isMetadataValid()==true")) {
                return 1;
            }
        }
    }

    // ── NRRD loader via LoadRequest (IVolumeLoader interface) ─────────────────
    printSection("NrrdLoader via LoadRequest interface");
    {
        const std::string path = std::string(VNEIO_TESTDATA_DIR) + "/volumes/small3d.nrrd";
        std::ifstream probe(path);
        if (!probe.good()) {
            VNE_LOG_INFO << "  small3d.nrrd not present — skipping LoadRequest test";
        } else {
            probe.close();
            vne::image::NrrdLoader loader;
            vne::io::LoadRequest req;
            req.asset_type = vne::io::AssetType::eVolume;
            req.uri = path;
            auto result = loader.loadVolume(req);
            if (!check(result.ok(), "loadVolume(LoadRequest) ok")) {
                return 1;
            }
            printStatus(result.status);
            if (!check(!result.value.isEmpty(), "result.value is not empty")) {
                return 1;
            }
        }
    }

    // ── Synthetic MHD: write then load ────────────────────────────────────────
    printSection("Synthetic MHD file — write then load via MhdLoader");
    {
        // Build a minimal 3×3×3 uint8 volume
        vne::image::Volume src;
        src.dims[0] = src.dims[1] = src.dims[2] = 3;
        src.spacing[0] = 0.5f;
        src.spacing[1] = 0.5f;
        src.spacing[2] = 1.0f;
        src.origin[0] = 1.0f;
        src.origin[1] = 2.0f;
        src.origin[2] = 3.0f;
        src.pixel_type = vne::image::VolumePixelType::eUint8;
        src.components = 1;
        src.data.resize(src.byteCount());
        for (size_t i = 0; i < src.data.size(); ++i) {
            src.data[i] = static_cast<uint8_t>(i * 10);
        }

        const std::string mhd_path = tmpPath("vneio_ex03_synth.mhd");
        std::string err;
        if (!check(vne::image::exportMhd(mhd_path, src, {}, &err), ("exportMhd to " + mhd_path).c_str())) {
            VNE_LOG_ERROR << "exportMhd error: " << err;
            return 1;
        }

        vne::image::MhdLoader loader;
        vne::image::Volume loaded;
        bool ok = loader.load(mhd_path, loaded);
        if (!check(ok, ("load MHD: " + mhd_path).c_str())) {
            VNE_LOG_ERROR << "MhdLoader error: " << loader.getLastError();
            return 1;
        }
        printVolumeInfo(loaded, "synthetic MHD (loaded)");
        if (!check(loaded.dims[0] == 3, "dims[0]==3")) {
            return 1;
        }
        if (!check(loaded.dims[1] == 3, "dims[1]==3")) {
            return 1;
        }
        if (!check(loaded.dims[2] == 3, "dims[2]==3")) {
            return 1;
        }
        if (!check(loaded.isMetadataValid(), "isMetadataValid()==true")) {
            return 1;
        }
    }

    // ── MhdLoader via LoadRequest interface ───────────────────────────────────
    printSection("MhdLoader via LoadRequest interface");
    {
        const std::string mhd_path = tmpPath("vneio_ex03_synth.mhd");
        std::ifstream probe(mhd_path);
        if (!probe.good()) {
            VNE_LOG_INFO << "  synthetic MHD not present — skipping LoadRequest test";
        } else {
            probe.close();
            vne::image::MhdLoader loader;
            vne::io::LoadRequest req;
            req.asset_type = vne::io::AssetType::eVolume;
            req.uri = mhd_path;
            auto result = loader.loadVolume(req);
            if (!check(result.ok(), "MhdLoader loadVolume(LoadRequest) ok")) {
                return 1;
            }
            printStatus(result.status);
        }
    }

    // ── Error path: non-existent file ─────────────────────────────────────────
    printSection("Error path: non-existent volume file");
    {
        vne::image::NrrdLoader loader;
        vne::image::Volume vol;
        bool ok = loader.load("/nonexistent/path/vol.nrrd", vol);
        if (!check(!ok, "load returns false for missing file")) {
            return 1;
        }
        if (!check(!loader.getLastError().empty(), "getLastError() non-empty")) {
            return 1;
        }
        VNE_LOG_ERROR << "  " << loader.getLastError();
    }

    VNE_LOG_INFO << "03_volume_loading: done.";
    return 0;
}

}  // namespace vne::io::examples
