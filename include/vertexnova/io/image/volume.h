#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * 3D volume type for medical/imaging data. Holds dimensions, spacing,
 * origin, pixel type, and raw buffer. Used by NRRD, MHD, and optionally
 * DICOM loaders.
 * ----------------------------------------------------------------------
 */

#include <cstdint>
#include <cstddef>
#include <vector>

namespace vne {
namespace Image {

/**
 * @brief Scalar pixel/voxel type for volumes
 */
enum class VolumePixelType : int {
    eUnknown = -1,
    eUint8 = 0,
    eInt8,
    eUint16,
    eInt16,
    eUint32,
    eInt32,
    eFloat32,
    eFloat64,
};

/**
 * @brief Bytes per voxel for each VolumePixelType
 */
inline int bytesPerVoxel(VolumePixelType t) {
    switch (t) {
        case VolumePixelType::eUint8:
            return 1;
        case VolumePixelType::eInt8:
            return 1;
        case VolumePixelType::eUint16:
            return 2;
        case VolumePixelType::eInt16:
            return 2;
        case VolumePixelType::eUint32:
            return 4;
        case VolumePixelType::eInt32:
            return 4;
        case VolumePixelType::eFloat32:
            return 4;
        case VolumePixelType::eFloat64:
            return 8;
        case VolumePixelType::eUnknown:
            return 0;
    }
    return 0;
}

/**
 * @brief 3D volume for medical/imaging data
 *
 * Dimensions (width, height, depth), spacing (mm or physical units),
 * origin, pixel type, and contiguous raw buffer. Enables multiplanar
 * reformats and window/level in viewers.
 */
struct Volume {
    int dims[3] = {0, 0, 0};  // width (x), height (y), depth (z)
    float spacing[3] = {1.0f, 1.0f, 1.0f};
    float origin[3] = {0.0f, 0.0f, 0.0f};
    float direction[9] = {
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
    VolumePixelType pixel_type = VolumePixelType::eUint8;
    int components = 1;
    std::vector<uint8_t> data;

    int width() const { return dims[0]; }
    int height() const { return dims[1]; }
    int depth() const { return dims[2]; }

    size_t voxelCount() const {
        return static_cast<size_t>(dims[0]) * static_cast<size_t>(dims[1]) * static_cast<size_t>(dims[2]);
    }

    size_t byteCount() const {
        return voxelCount() * static_cast<size_t>(components) * static_cast<size_t>(bytesPerVoxel(pixel_type));
    }

    bool isEmpty() const { return dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0 || data.size() < byteCount(); }

    const uint8_t* getData() const { return data.data(); }
    uint8_t* getData() { return data.data(); }
};

/** @brief Canonical CPU volume type alias (for AssetIO / upload documentation). */
using VolumeAsset = Volume;

}  // namespace Image
}  // namespace vne
