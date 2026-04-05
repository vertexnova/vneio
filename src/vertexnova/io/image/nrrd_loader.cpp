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
#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <string>

#include <NrrdIO.h>

namespace {

CREATE_VNE_LOGGER_CATEGORY("vne.io.image.nrrd_loader");

constexpr float kSpaceDirectionMinLength = 1e-20f;
/** Relative tolerance when comparing NRRD axis spacing to |space direction|. */
constexpr float kSpacingVersusDirectionLengthRelTol = 1e-3f;
constexpr std::size_t kNrrdLoadErrorLogMaxChars = 200;
constexpr int kDirMatElems = vne::image::kVolumeDirectionMatrixElements;
constexpr float kIdentityDirection3x3[kDirMatElems] = {
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
};

}  // namespace

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
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    char* err = nullptr;
    if (nrrdLoad(nin, const_cast<char*>(path.c_str()), nullptr)) {
        err = biffGetDone(NRRD);
        last_error_ = std::string("NrrdLoader: ") + (err ? err : "unknown error");
        if (err) {
            free(err);
        }
        VNE_LOG_ERROR << "NrrdLoader: load failed for \"" << path << "\": "
                      << (last_error_.size() > kNrrdLoadErrorLogMaxChars
                              ? last_error_.substr(0, kNrrdLoadErrorLogMaxChars) + "..."
                              : last_error_);
        nrrdNuke(nin);
        return false;
    }

    VNE_LOG_INFO << "NrrdLoader: loading \"" << path << "\"";

    if (nin->dim < 1 || nin->dim > 3) {
        last_error_ = "NrrdLoader: dimension 1, 2, or 3 supported, got " + std::to_string(nin->dim);
        VNE_LOG_ERROR << last_error_;
        nrrdNuke(nin);
        return false;
    }

    for (unsigned int i = 0; i < nin->dim; ++i) {
        unsigned int ks = nrrdKindSize(nin->axis[i].kind);
        if (ks >= 2) {
            last_error_ = "NrrdLoader: multi-component / non-scalar axis kind not supported (axis " + std::to_string(i)
                          + ", kind " + std::to_string(nin->axis[i].kind) + ")";
            VNE_LOG_ERROR << last_error_;
            nrrdNuke(nin);
            return false;
        }
    }

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
            VNE_LOG_ERROR << last_error_;
            nrrdNuke(nin);
            return false;
    }

    int sizes[3] = {1, 1, 1};
    for (unsigned int i = 0; i < nin->dim && i < 3u; ++i) {
        sizes[i] = static_cast<int>(nin->axis[i].size);
    }
    if (sizes[0] <= 0 || sizes[1] <= 0 || sizes[2] <= 0) {
        last_error_ = "NrrdLoader: invalid sizes";
        VNE_LOG_ERROR << last_error_;
        nrrdNuke(nin);
        return false;
    }

    out_volume.dims[0] = sizes[0];
    out_volume.dims[1] = sizes[1];
    out_volume.dims[2] = sizes[2];
    out_volume.pixel_type = pixel_type;
    out_volume.components = 1;

    const size_t expected_elements = nrrdElementNumber(nin);
    const size_t expected_bytes = nrrdElementSize(nin) * expected_elements;
    const size_t vol_bytes = out_volume.byteCount();
    if (expected_bytes != vol_bytes) {
        last_error_ = "NrrdLoader: data size mismatch (file " + std::to_string(expected_bytes) + " bytes, volume "
                      + std::to_string(vol_bytes) + ")";
        VNE_LOG_ERROR << last_error_;
        nrrdNuke(nin);
        return false;
    }

    if (nin->spaceDim > 0 && nin->spaceDim <= 3) {
        for (unsigned int i = 0; i < nin->spaceDim; ++i) {
            out_volume.origin[i] = static_cast<float>(nin->spaceOrigin[i]);
        }
    }

    std::memcpy(out_volume.direction, kIdentityDirection3x3, sizeof(kIdentityDirection3x3));

    if (nin->spaceDim == 3) {
        for (int i = 0; i < kVolumeSpatialDim; ++i) {
            if (i >= static_cast<int>(nin->dim)) {
                continue;
            }
            const double dx = nin->axis[i].spaceDirection[0];
            const double dy = nin->axis[i].spaceDirection[1];
            const double dz = nin->axis[i].spaceDirection[2];
            const bool sd_valid = !std::isnan(dx) && !std::isnan(dy) && !std::isnan(dz) && !std::isinf(dx)
                                  && !std::isinf(dy) && !std::isinf(dz);
            if (sd_valid) {
                const auto fx = static_cast<float>(dx);
                const auto fy = static_cast<float>(dy);
                const auto fz = static_cast<float>(dz);
                const float len = std::sqrt(fx * fx + fy * fy + fz * fz);
                if (len > kSpaceDirectionMinLength) {
                    out_volume.spacing[i] = len;
                    out_volume.direction[volumeDirectionIndex(i, 0)] = fx / len;
                    out_volume.direction[volumeDirectionIndex(i, 1)] = fy / len;
                    out_volume.direction[volumeDirectionIndex(i, 2)] = fz / len;
                    const bool spacing_set = !std::isnan(nin->axis[i].spacing) && nin->axis[i].spacing > 0;
                    if (spacing_set) {
                        const auto from_axis = static_cast<float>(nin->axis[i].spacing);
                        if (std::fabs(from_axis - len) > kSpacingVersusDirectionLengthRelTol * std::max(len, 1.0f)) {
                            VNE_LOG_WARN << "NrrdLoader: axis " << i
                                         << " spacing field disagrees with |space direction|; using |space direction|="
                                         << len << " (spacing field was " << from_axis << ")";
                        }
                    }
                    VNE_LOG_DEBUG << "NrrdLoader: axis " << i << " space direction length=" << len << " (normalized)";
                } else {
                    VNE_LOG_WARN << "NrrdLoader: axis " << i
                                 << " space direction near zero; using identity row + axis "
                                    "spacing fallback";
                    out_volume.direction[volumeDirectionIndex(i, 0)] =
                        kIdentityDirection3x3[volumeDirectionIndex(i, 0)];
                    out_volume.direction[volumeDirectionIndex(i, 1)] =
                        kIdentityDirection3x3[volumeDirectionIndex(i, 1)];
                    out_volume.direction[volumeDirectionIndex(i, 2)] =
                        kIdentityDirection3x3[volumeDirectionIndex(i, 2)];
                    if (!std::isnan(nin->axis[i].spacing) && nin->axis[i].spacing > 0) {
                        out_volume.spacing[i] = static_cast<float>(nin->axis[i].spacing);
                    }
                }
            } else {
                if (!std::isnan(nin->axis[i].spacing) && nin->axis[i].spacing > 0) {
                    out_volume.spacing[i] = static_cast<float>(nin->axis[i].spacing);
                }
                VNE_LOG_DEBUG << "NrrdLoader: axis " << i
                              << " no valid space direction; spacing=" << out_volume.spacing[i];
            }
        }
    } else {
        for (unsigned int i = 0; i < nin->dim && i < static_cast<unsigned int>(kVolumeSpatialDim); ++i) {
            if (!std::isnan(nin->axis[i].spacing) && nin->axis[i].spacing > 0) {
                out_volume.spacing[i] = static_cast<float>(nin->axis[i].spacing);
            }
        }
        VNE_LOG_DEBUG << "NrrdLoader: spaceDim=" << nin->spaceDim << " (no 3D space directions); using spacings only";
    }

    out_volume.data.resize(vol_bytes);
    std::memcpy(out_volume.data.data(), nin->data, vol_bytes);

    if (!out_volume.hasExactBufferSize()) {
        last_error_ = "NrrdLoader: internal buffer size error";
        VNE_LOG_ERROR << last_error_;
        nrrdNuke(nin);
        return false;
    }

    nrrdNuke(nin);

    VNE_LOG_INFO << "NrrdLoader: loaded \"" << path << "\" dims=" << out_volume.dims[0] << "x" << out_volume.dims[1]
                 << "x" << out_volume.dims[2] << " type=" << static_cast<int>(out_volume.pixel_type) << " spacing=("
                 << out_volume.spacing[0] << "," << out_volume.spacing[1] << "," << out_volume.spacing[2]
                 << ") origin=(" << out_volume.origin[0] << "," << out_volume.origin[1] << "," << out_volume.origin[2]
                 << ")";

    return true;
}

}  // namespace image
}  // namespace vne
