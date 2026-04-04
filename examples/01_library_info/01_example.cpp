/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 01: Library info — enumerate supported formats, pixel types,
 * loader capabilities, and error codes. No file I/O.
 * ----------------------------------------------------------------------
 */

#include "01_example.h"
#include "example_utils.h"

#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/common/status.h"

namespace vne::io::examples {

int runLibraryInfoExample() {
    LoggingGuard logging_guard;

    // ── Image formats ─────────────────────────────────────────────────────────
    printSection("Image Formats (StbImageLoader)");
    {
        vne::image::StbImageLoader loader;
        const char* exts[] = { "png", "jpg", "jpeg", "bmp", "tga", "hdr", "nrrd", "mhd" };
        for (const char* ext : exts) {
            std::string dummy = std::string("file.") + ext;
            VNE_LOG_INFO << "  " << dummy << " -> "
                << (loader.isExtensionSupported(dummy) ? "supported" : "not supported");
        }
    }

    // ── Volume formats ────────────────────────────────────────────────────────
    printSection("Volume Formats");
    {
        vne::image::NrrdLoader nrrd;
        vne::image::MhdLoader  mhd;

        const char* exts[] = { "nrrd", "nhdr", "mhd", "mha", "obj" };
        VNE_LOG_INFO << "NrrdLoader:";
        for (const char* ext : exts) {
            std::string dummy = std::string("vol.") + ext;
            VNE_LOG_INFO << "  " << dummy << " -> "
                << (nrrd.isExtensionSupported(dummy) ? "supported" : "not supported");
        }
        VNE_LOG_INFO << "MhdLoader:";
        for (const char* ext : exts) {
            std::string dummy = std::string("vol.") + ext;
            VNE_LOG_INFO << "  " << dummy << " -> "
                << (mhd.isExtensionSupported(dummy) ? "supported" : "not supported");
        }
    }

    // ── Mesh formats ──────────────────────────────────────────────────────────
    printSection("Mesh Formats (AssimpLoader)");
    {
        vne::mesh::AssimpLoader loader;
        const char* exts[] = { "obj", "stl", "fbx", "gltf", "glb", "ply", "dae", "nrrd" };
        for (const char* ext : exts) {
            std::string dummy = std::string("mesh.") + ext;
            VNE_LOG_INFO << "  " << dummy << " -> "
                << (loader.isExtensionSupported(dummy) ? "supported" : "not supported");
        }
    }

    // ── VolumePixelType table ─────────────────────────────────────────────────
    printSection("VolumePixelType — bytes per voxel");
    {
        using T = vne::image::VolumePixelType;
        const T types[] = {
            T::eUint8, T::eInt8, T::eUint16, T::eInt16,
            T::eUint32, T::eInt32, T::eFloat32, T::eFloat64, T::eUnknown
        };
        for (auto t : types) {
            VNE_LOG_INFO << "  " << pixelTypeName(t)
                << " -> " << vne::image::bytesPerVoxel(t) << " bytes";
        }
    }

    // ── ErrorCode table ───────────────────────────────────────────────────────
    printSection("ErrorCode values");
    {
        const ErrorCode codes[] = {
            ErrorCode::eOk, ErrorCode::eUnknown, ErrorCode::eInvalidArgument,
            ErrorCode::eNotImplemented, ErrorCode::eOutOfMemory,
            ErrorCode::eFileNotFound, ErrorCode::eFileOpenFailed,
            ErrorCode::eFileReadFailed, ErrorCode::eFileWriteFailed,
            ErrorCode::ePathInvalid, ErrorCode::eUnsupportedFormat,
            ErrorCode::eUnsupportedFeature, ErrorCode::eParseError,
            ErrorCode::eDataCorrupt, ErrorCode::eDataTruncated,
            ErrorCode::eInvalidDimensions, ErrorCode::eInvalidPixelType,
            ErrorCode::eThirdPartyError
        };
        for (auto c : codes) {
            VNE_LOG_INFO << "  " << static_cast<int>(c) << "  " << errorCodeName(c);
        }
    }

    // ── Default Volume geometry ───────────────────────────────────────────────
    printSection("Default Volume (zero-constructed)");
    {
        vne::image::Volume v;
        printVolumeInfo(v);
        if (!check(v.isEmpty(),            "empty volume isEmpty()==true"))        return 1;
        if (!check(!v.isMetadataValid(),   "empty volume isMetadataValid()==false")) return 1;
        if (!check(v.hasIdentityDirection(),"default direction is identity"))       return 1;
    }

    VNE_LOG_INFO << "01_library_info: done.";
    return 0;
}

}  // namespace vne::io::examples
