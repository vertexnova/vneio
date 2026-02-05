#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include <cstdint>
#include <cstddef>
#include <vector>

namespace vne {
namespace image {

/**
 * @file volume.h
 * @brief 3D volume type for medical/imaging data (dimensions, spacing, origin, pixel type, buffer).
 */

/**
 * @enum VolumePixelType
 * @brief Scalar pixel/voxel type for volumes.
 */
enum class VolumePixelType : int {
    eUnknown = -1,  //!< Unknown or unsupported type.
    eUint8 = 0,     //!< 8-bit unsigned.
    eInt8,          //!< 8-bit signed.
    eUint16,        //!< 16-bit unsigned.
    eInt16,         //!< 16-bit signed.
    eUint32,        //!< 32-bit unsigned.
    eInt32,         //!< 32-bit signed.
    eFloat32,       //!< 32-bit float.
    eFloat64,       //!< 64-bit float.
};

/** Number of elements in a 3x3 direction matrix (row-major). */
constexpr int kVolumeDirectionMatrixElements = 9;

/** Bytes per voxel for VolumePixelType::eFloat64. */
constexpr int kBytesPerFloat64 = 8;

/**
 * @brief Bytes per voxel for the given VolumePixelType.
 * @param t Pixel type.
 * @return Byte count (0 for eUnknown).
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
 * @struct Volume
 * @brief 3D volume for medical/imaging data.
 *
 * Dimensions (width, height, depth), spacing (mm or physical units),
 * origin, pixel type, and contiguous raw buffer. Used for multiplanar
 * reformats and window/level in viewers.
 */
struct Volume {
    int dims[3] = {0, 0, 0};   //!< Width (x), height (y), depth (z).
    float spacing[3] = {1.0f, 1.0f, 1.0f};  //!< Voxel spacing (e.g. mm).
    float origin[3] = {0.0f, 0.0f, 0.0f};   //!< World-space origin.
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
    VolumePixelType pixel_type = VolumePixelType::eUint8;  //!< Scalar type of voxels.
    int components = 1;   //!< Components per voxel (1 for scalar).
    std::vector<uint8_t> data;  //!< Contiguous voxel data.

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
