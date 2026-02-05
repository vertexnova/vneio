/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"
#include <NrrdIO.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace vne {
namespace image {

bool NrrdLoader::canLoad(const vne::io::LoadRequest& request) const {
    if (request.asset_type != vne::io::AssetType::eVolume) {
        return false;
    }
    return isExtensionSupported(request.uri);
}

vne::io::LoadResult<vne::image::Volume> NrrdLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<vne::image::Volume> result;
    if (!load(request.uri, result.value)) {
        result.status =
            vne::io::Status::make(vne::io::ErrorCode::eParseError, getLastError(), request.uri, "NrrdLoader");
        return result;
    }
    result.status = vne::io::Status::okStatus();
    return result;
}

bool NrrdLoader::isExtensionSupported(const std::string& path) const {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return false;
    }
    std::string ext = path.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".nrrd" || ext == ".nhdr";
}

bool NrrdLoader::load(const std::string& path, Volume& out_volume) {
    last_error_.clear();
    out_volume = Volume{};

    Nrrd* nin = nrrdNew();
    if (!nin) {
        last_error_ = "NrrdLoader: failed to create Nrrd struct";
        return false;
    }

    char* err = nullptr;
    if (nrrdLoad(nin, const_cast<char*>(path.c_str()), nullptr)) {
        err = biffGetDone(NRRD);
        last_error_ = std::string("NrrdLoader: ") + (err ? err : "unknown error");
        if (err) {
            free(err);
        }
        nrrdNuke(nin);
        return false;
    }

    // Support 1D, 2D, or 3D; store as 3D volume (unused dims = 1)
    if (nin->dim < 1 || nin->dim > 3) {
        last_error_ = "NrrdLoader: dimension 1, 2, or 3 supported, got " + std::to_string(nin->dim);
        nrrdNuke(nin);
        return false;
    }

    // Map nrrdType to VolumePixelType
    VolumePixelType pixel_type = VolumePixelType::eUnknown;
    switch (nin->type) {
        case nrrdTypeUChar:
            pixel_type = VolumePixelType::eUint8;
            break;
        case nrrdTypeChar:
            pixel_type = VolumePixelType::eInt8;
            break;
        case nrrdTypeUShort:
            pixel_type = VolumePixelType::eUint16;
            break;
        case nrrdTypeShort:
            pixel_type = VolumePixelType::eInt16;
            break;
        case nrrdTypeUInt:
            pixel_type = VolumePixelType::eUint32;
            break;
        case nrrdTypeInt:
            pixel_type = VolumePixelType::eInt32;
            break;
        case nrrdTypeFloat:
            pixel_type = VolumePixelType::eFloat32;
            break;
        case nrrdTypeDouble:
            pixel_type = VolumePixelType::eFloat64;
            break;
        default:
            last_error_ = "NrrdLoader: unsupported pixel type";
            nrrdNuke(nin);
            return false;
    }

    // Extract dimensions (axis[0] fastest, axis[2] slowest); pad with 1 for 1D/2D
    int sizes[3] = {1, 1, 1};
    for (unsigned int i = 0; i < nin->dim && i < 3u; ++i) {
        sizes[i] = static_cast<int>(nin->axis[i].size);
    }
    if (sizes[0] <= 0 || sizes[1] <= 0 || sizes[2] <= 0) {
        last_error_ = "NrrdLoader: invalid sizes";
        nrrdNuke(nin);
        return false;
    }

    out_volume.dims[0] = sizes[0];
    out_volume.dims[1] = sizes[1];
    out_volume.dims[2] = sizes[2];
    out_volume.pixel_type = pixel_type;

    // Extract spacing (if available); only set for present axes
    for (unsigned int i = 0; i < nin->dim && i < 3u; ++i) {
        if (!std::isnan(nin->axis[i].spacing) && nin->axis[i].spacing > 0) {
            out_volume.spacing[i] = static_cast<float>(nin->axis[i].spacing);
        }
    }

    // Extract origin (if space information is available)
    if (nin->spaceDim > 0 && nin->spaceDim <= 3) {
        for (unsigned int i = 0; i < nin->spaceDim; ++i) {
            out_volume.origin[i] = static_cast<float>(nin->spaceOrigin[i]);
        }
    }

    // Extract direction matrix (if available)
    if (nin->spaceDim == 3) {
        for (int i = 0; i < 3; ++i) {
            if (!std::isnan(nin->axis[i].spaceDirection[0]) && !std::isnan(nin->axis[i].spaceDirection[1])
                && !std::isnan(nin->axis[i].spaceDirection[2])) {
                out_volume.direction[i * 3 + 0] = static_cast<float>(nin->axis[i].spaceDirection[0]);
                out_volume.direction[i * 3 + 1] = static_cast<float>(nin->axis[i].spaceDirection[1]);
                out_volume.direction[i * 3 + 2] = static_cast<float>(nin->axis[i].spaceDirection[2]);
            }
        }
    }

    // Copy data
    size_t num_bytes = out_volume.byteCount();
    out_volume.data.resize(num_bytes);
    std::memcpy(out_volume.data.data(), nin->data, num_bytes);

    nrrdNuke(nin);
    return true;
}

}  // namespace image
}  // namespace vne
