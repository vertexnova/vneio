/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 06: Unified AssetIO registry — register all loaders, load
 * image / volume / mesh through a single interface, demonstrate the
 * LoadResult<T> pattern, and verify error codes on negative paths.
 *
 * This is the pattern a multibackend renderer host would use to load
 * all scene assets before uploading to GPU.
 * ----------------------------------------------------------------------
 */

#include "06_example.h"
#include "example_utils.h"

#include "vertexnova/io/asset_io.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/mesh/assimp_loader.h"

#include <memory>
#include <string>

#ifndef VNEIO_TESTDATA_DIR
#define VNEIO_TESTDATA_DIR "testdata"
#endif

namespace vne::io::examples {

int runAssetRegistryExample() {
    LoggingGuard logging_guard;

    // ── Build registry ────────────────────────────────────────────────────────
    printSection("Build AssetIO registry");
    {
        vne::io::AssetIO registry;
        registry.registerImageLoader(std::make_unique<vne::image::StbImageLoader>());
        registry.registerVolumeLoader(std::make_unique<vne::image::NrrdLoader>());
        registry.registerVolumeLoader(std::make_unique<vne::image::MhdLoader>());
        registry.registerMeshLoader(std::make_unique<vne::mesh::AssimpLoader>());
        if (!check(true, "registry constructed and loaders registered")) return 1;
    }

    // ── Shared registry for remaining sections ────────────────────────────────
    vne::io::AssetIO registry;
    registry.registerImageLoader(std::make_unique<vne::image::StbImageLoader>());
    registry.registerVolumeLoader(std::make_unique<vne::image::NrrdLoader>());
    registry.registerVolumeLoader(std::make_unique<vne::image::MhdLoader>());
    registry.registerMeshLoader(std::make_unique<vne::mesh::AssimpLoader>());

    // ── Load image ────────────────────────────────────────────────────────────
    printSection("Load image via registry");
    {
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eImage;
        req.uri        = std::string(VNEIO_TESTDATA_DIR) + "/textures/sample.png";

        auto result = registry.loadImage(req);
        printStatus(result.status);
        if (result.ok()) {
            if (!check(!result.value.isEmpty(), "image is not empty")) return 1;
            VNE_LOG_INFO << "  image: " << result.value.getWidth()
                << "x" << result.value.getHeight()
                << " ch=" << result.value.getChannels();
        } else if (result.status.code == vne::io::ErrorCode::eFileNotFound) {
            VNE_LOG_WARN << "testdata not found — continuing with remaining sections";
        } else {
            VNE_LOG_ERROR << "unexpected error loading image";
            return 1;
        }
    }

    // ── Load volume ───────────────────────────────────────────────────────────
    printSection("Load volume via registry");
    {
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eVolume;
        req.uri        = std::string(VNEIO_TESTDATA_DIR) + "/volumes/small3d.nrrd";

        auto result = registry.loadVolume(req);
        printStatus(result.status);
        if (result.ok()) {
            if (!check(!result.value.isEmpty(),        "volume is not empty"))     return 1;
            if (!check(result.value.isMetadataValid(), "isMetadataValid()==true")) return 1;
            printVolumeInfo(result.value);
        } else if (result.status.code == vne::io::ErrorCode::eFileNotFound) {
            VNE_LOG_WARN << "testdata not found — continuing";
        } else {
            VNE_LOG_ERROR << "unexpected error loading volume";
            return 1;
        }
    }

    // ── Load mesh ─────────────────────────────────────────────────────────────
    printSection("Load mesh via registry");
    {
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eMesh;
        req.uri        = std::string(VNEIO_TESTDATA_DIR) + "/meshes/minimal.stl";

        auto result = registry.loadMesh(req);
        printStatus(result.status);
        if (result.ok()) {
            if (!check(!result.value.isEmpty(), "mesh is not empty")) return 1;
            printMeshInfo(result.value);
        } else if (result.status.code == vne::io::ErrorCode::eFileNotFound) {
            VNE_LOG_WARN << "testdata not found — continuing";
        } else {
            VNE_LOG_ERROR << "unexpected error loading mesh";
            return 1;
        }
    }

    // ── LoadResult<T> pattern demo ────────────────────────────────────────────
    printSection("LoadResult<T> pattern: ok() gate before value access");
    {
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eVolume;
        req.uri        = "/nonexistent/path/vol.nrrd";

        auto result = registry.loadVolume(req);
        // Always gate on ok() before accessing .value
        if (result.ok()) {
            VNE_LOG_ERROR << "expected failure but got ok — something is wrong";
            return 1;
        }
        if (!check(!result.ok(), "result.ok()==false for missing file")) return 1;
        VNE_LOG_INFO << "  code=" << errorCodeName(result.status.code)
            << "  subsystem=" << result.status.subsystem
            << "  msg=" << result.status.message;
    }

    // ── Negative: file not found ──────────────────────────────────────────────
    printSection("Negative: eFileNotFound for missing image");
    {
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eImage;
        req.uri        = "/nonexistent/path/image.png";

        auto result = registry.loadImage(req);
        if (!check(!result.ok(), "result not ok")) return 1;
        VNE_LOG_INFO << "  code=" << errorCodeName(result.status.code);
    }

    // ── Negative: unsupported format ──────────────────────────────────────────
    printSection("Negative: eUnsupportedFormat when no loader matches");
    {
        // Build a registry with no loaders
        vne::io::AssetIO empty_registry;

        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eImage;
        req.uri        = "something.png";

        auto result = empty_registry.loadImage(req);
        if (!check(!result.ok(), "empty registry result not ok")) return 1;
        if (!check(result.status.code == vne::io::ErrorCode::eUnsupportedFormat,
                   "code == eUnsupportedFormat")) return 1;
        VNE_LOG_INFO << "  code=" << errorCodeName(result.status.code);
    }

    // ── Negative: unsupported format for mesh ─────────────────────────────────
    printSection("Negative: eUnsupportedFormat for mesh in empty registry");
    {
        vne::io::AssetIO empty_registry;
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eMesh;
        req.uri        = "mesh.stl";
        auto result    = empty_registry.loadMesh(req);
        if (!check(!result.ok(), "empty registry mesh result not ok")) return 1;
        if (!check(result.status.code == vne::io::ErrorCode::eUnsupportedFormat,
                   "code == eUnsupportedFormat")) return 1;
        VNE_LOG_INFO << "  code=" << errorCodeName(result.status.code);
    }

    VNE_LOG_INFO << "06_asset_registry: done.";
    return 0;
}

}  // namespace vne::io::examples
