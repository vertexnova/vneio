#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include <cstdint>
#include <cstddef>
#include <vector>

namespace vne {
namespace image {

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

/** Number of elements in a 3x3 direction matrix (row-major). */
constexpr int kVolumeDirectionMatrixElements = 9;

/** Bytes per voxel for VolumePixelType::eFloat64. */
constexpr int kBytesPerFloat64 = 8;

/**
 * @brief Bytes per voxel for each VolumePixelType
 */
[[nodiscard]] inline int bytesPerVoxel(VolumePixelType t) {
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
            return kBytesPerFloat64;
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
    float direction[kVolumeDirectionMatrixElements] = {
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

    [[nodiscard]] int width() const { return dims[0]; }
    [[nodiscard]] int height() const { return dims[1]; }
    [[nodiscard]] int depth() const { return dims[2]; }

    [[nodiscard]] size_t voxelCount() const {
        return static_cast<size_t>(dims[0]) * static_cast<size_t>(dims[1]) * static_cast<size_t>(dims[2]);
    }

    [[nodiscard]] size_t byteCount() const {
        return voxelCount() * static_cast<size_t>(components) * static_cast<size_t>(bytesPerVoxel(pixel_type));
    }

    [[nodiscard]] bool isEmpty() const { return dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0 || data.size() < byteCount(); }

    [[nodiscard]] const uint8_t* getData() const { return data.data(); }
    [[nodiscard]] uint8_t* getData() { return data.data(); }
};

/** @brief Canonical CPU volume type alias (for AssetIO / upload documentation). */
using VolumeAsset = Volume;

}  // namespace image
}  // namespace vne
