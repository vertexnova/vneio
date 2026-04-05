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

#include "vertexnova/io/export.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
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

/** Spatial axis count (MetaImage / NRRD-style 3D volumes). */
constexpr int kVolumeSpatialDim = 3;

/** Row-major linear index into @c Volume::direction; @a row and @a col are in `[0, kVolumeSpatialDim)`. */
[[nodiscard]] constexpr int volumeDirectionIndex(int row, int col) noexcept {
    return row * kVolumeSpatialDim + col;
}

/** Bytes per voxel for VolumePixelType::eFloat64. */
constexpr int kBytesPerFloat64 = 8;

/** Epsilon for direction / metadata checks. */
constexpr float kVolumeDirectionEpsilon = 1e-4f;
/** Minimum acceptable row length before treating a direction row as degenerate. */
constexpr float kVolumeDirectionRowMinLen = 0.01f;
/** Maximum deviation of row length from 1 for @ref isMetadataValid. */
constexpr float kVolumeDirectionRowUnitSlack = 0.02f;
/** Minimum |det(direction)| for a well-conditioned basis in @ref isMetadataValid. */
constexpr float kVolumeDirectionMinDeterminant = 1e-4f;

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
 * **Geometry convention (for I/O and rendering):** @a spacing[i] is the sample distance
 * along axis i. @a direction stores a row-major 3×3 matrix whose rows are **unit** axis
 * direction vectors (world space). NRRD writers combine them as `space directions` =
 * `direction[row] * spacing[row]`. Loaders must normalize NRRD space directions into this form.
 *
 * Dimensions (width, height, depth), spacing, origin, pixel type, and contiguous raw buffer.
 *
 * **Typed access:** @c std::vector<uint8_t> storage is only byte-aligned. There is no @c dataAs()
 * returning @c T* (that pattern was removed). Do not @c reinterpret_cast the buffer to @c T* and
 * dereference for multi-byte @c T — that can be UB on strict-alignment platforms. Use @ref
 * readVoxelAt for @c memcpy-based scalar reads, @ref byteSpan / @ref rawByteSpan / @ref getData for
 * raw bytes, or @c memcpy in your own code. An aligned custom allocator for @c data is optional
 * and not required when using @ref readVoxelAt.
 */
struct VNEIO_API Volume {
    int dims[3] = {0, 0, 0};                //!< Width (x), height (y), depth (z).
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
    int components = 1;                                    //!< Components per voxel (1 for scalar).
    std::vector<uint8_t> data;                             //!< Contiguous voxel data.

    [[nodiscard]] int width() const { return dims[0]; }
    [[nodiscard]] int height() const { return dims[1]; }
    [[nodiscard]] int depth() const { return dims[2]; }

    [[nodiscard]] size_t voxelCount() const {
        return static_cast<size_t>(dims[0]) * static_cast<size_t>(dims[1]) * static_cast<size_t>(dims[2]);
    }

    [[nodiscard]] size_t byteCount() const {
        return voxelCount() * static_cast<size_t>(components) * static_cast<size_t>(bytesPerVoxel(pixel_type));
    }

    [[nodiscard]] bool isEmpty() const {
        return dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0 || data.size() < byteCount();
    }

    /** True if dimensions are positive and the buffer size matches @ref byteCount() exactly. */
    [[nodiscard]] bool hasExactBufferSize() const {
        if (dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0) {
            return false;
        }
        if (components <= 0 || pixel_type == VolumePixelType::eUnknown) {
            return false;
        }
        return data.size() == byteCount();
    }

    [[nodiscard]] bool hasScalarVoxels() const { return components == 1; }

    /** True if @a direction is approximately the 3×3 identity. */
    [[nodiscard]] bool hasIdentityDirection() const {
        const float e = kVolumeDirectionEpsilon;
        for (int r = 0; r < kVolumeSpatialDim; ++r) {
            for (int c = 0; c < kVolumeSpatialDim; ++c) {
                const float expected = (r == c) ? 1.0f : 0.0f;
                if (std::fabs(direction[volumeDirectionIndex(r, c)] - expected) >= e) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @brief Read-only view of the raw voxel bytes (no alignment claim beyond byte-sized elements).
     */
    [[nodiscard]] std::span<const std::uint8_t> byteSpan() const noexcept { return {data.data(), data.size()}; }

    /**
     * @brief Same buffer as @ref byteSpan as @c std::byte (object-representation view).
     */
    [[nodiscard]] std::span<const std::byte> rawByteSpan() const noexcept { return std::as_bytes(byteSpan()); }

    /**
     * @brief Copy one stored value of type @a T from the raw buffer at logical index @a linear_index.
     *
     * Interprets the buffer as a contiguous array of `T` (stride `sizeof(T)` bytes). For scalar
     * volumes, @a linear_index is the flat voxel index; multi-component layouts are the caller's
     * responsibility.
     *
     * Uses `memcpy`, so no alignment requirement on `data.data()`. If the slice would extend past
     * `data.size()`, returns a value-initialized `T`.
     */
    template<typename T>
    [[nodiscard]] T readVoxelAt(size_t linear_index) const {
        static_assert(std::is_trivially_copyable_v<T>);
        T out{};
        const size_t off = linear_index * sizeof(T);
        if (off + sizeof(T) > data.size()) {
            return out;
        }
        std::memcpy(&out, data.data() + off, sizeof(T));
        return out;
    }

    /**
     * @brief Heuristic validity for upload / rendering: positive dims, known scalar type,
     *        positive finite spacing, well-conditioned direction rows, exact buffer size.
     */
    [[nodiscard]] bool isMetadataValid() const {
        if (!hasExactBufferSize() || !hasScalarVoxels()) {
            return false;
        }
        for (int i = 0; i < kVolumeSpatialDim; ++i) {
            if (!std::isfinite(spacing[i]) || spacing[i] <= 0.0f) {
                return false;
            }
            if (!std::isfinite(origin[i])) {
                return false;
            }
        }
        auto row_len = [this](int row) {
            const int b = row * kVolumeSpatialDim;
            const float x = direction[b + 0];
            const float y = direction[b + 1];
            const float z = direction[b + 2];
            return std::sqrt(x * x + y * y + z * z);
        };
        for (int r = 0; r < kVolumeSpatialDim; ++r) {
            float len = row_len(r);
            if (!std::isfinite(len) || len < kVolumeDirectionRowMinLen
                || std::fabs(len - 1.0f) > kVolumeDirectionRowUnitSlack) {
                return false;
            }
        }
        const auto dir = [this](int row, int col) { return direction[volumeDirectionIndex(row, col)]; };
        const float det = dir(0, 0) * (dir(1, 1) * dir(2, 2) - dir(1, 2) * dir(2, 1))
                          - dir(0, 1) * (dir(1, 0) * dir(2, 2) - dir(1, 2) * dir(2, 0))
                          + dir(0, 2) * (dir(1, 0) * dir(2, 1) - dir(1, 1) * dir(2, 0));
        if (!std::isfinite(det) || std::fabs(det) < kVolumeDirectionMinDeterminant) {
            return false;
        }
        return true;
    }

    [[nodiscard]] const uint8_t* getData() const { return data.data(); }
    [[nodiscard]] uint8_t* getData() { return data.data(); }
};

/** @brief Canonical CPU volume type alias (for AssetIO / upload documentation). */
using VolumeAsset = Volume;

}  // namespace image
}  // namespace vne
