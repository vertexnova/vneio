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

namespace VNE {
namespace Image {

/**
 * @brief Scalar pixel/voxel type for volumes
 */
enum class VolumePixelType : int {
    eUint8 = 0,
    eUint16 = 1,
    eFloat32 = 2,
};

/**
 * @brief Bytes per voxel for each VolumePixelType
 */
inline int bytesPerVoxel(VolumePixelType t) {
    switch (t) {
        case VolumePixelType::eUint8: return 1;
        case VolumePixelType::eUint16: return 2;
        case VolumePixelType::eFloat32: return 4;
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
    int dims[3] = {0, 0, 0};       // width (x), height (y), depth (z)
    float spacing[3] = {1.0f, 1.0f, 1.0f};
    float origin[3] = {0.0f, 0.0f, 0.0f};
    VolumePixelType pixel_type = VolumePixelType::eUint8;
    std::vector<uint8_t> data;

    int width() const { return dims[0]; }
    int height() const { return dims[1]; }
    int depth() const { return dims[2]; }

    size_t voxelCount() const {
        return static_cast<size_t>(dims[0]) * static_cast<size_t>(dims[1]) * static_cast<size_t>(dims[2]);
    }

    size_t byteCount() const {
        return voxelCount() * static_cast<size_t>(bytesPerVoxel(pixel_type));
    }

    bool isEmpty() const {
        return dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0 || data.size() < byteCount();
    }

    const uint8_t* getData() const { return data.data(); }
    uint8_t* getData() { return data.data(); }
};

}  // namespace Image
}  // namespace VNE
