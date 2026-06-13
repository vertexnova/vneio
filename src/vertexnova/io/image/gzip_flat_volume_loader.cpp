/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/image/gzip_flat_volume_loader.h"

#include "vertexnova/io/common/compression.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace {

CREATE_VNE_LOGGER_CATEGORY("vne.io.image.gzip_flat_volume_loader")

constexpr std::size_t kBinGzSuffixLen = 7U;  // length of ".bin-gz"

[[nodiscard]] bool endsWithBinGz(const std::string& path) {
    if (path.size() < kBinGzSuffixLen) {
        return false;
    }
    const std::string tail = path.substr(path.size() - kBinGzSuffixLen);
    std::string lower = tail;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lower == ".bin-gz";
}

[[nodiscard]] std::size_t flatBytesPerVoxel(const vne::image::VolumePixelType type) {
    switch (type) {
        case vne::image::VolumePixelType::eUint8:
            return 1U;
        case vne::image::VolumePixelType::eUint16:
            return 2U;
        case vne::image::VolumePixelType::eFloat32:
            return 4U;
        default:
            return 0U;
    }
}

[[nodiscard]] bool parseDimensions(const std::string& token, vne::image::FlatVolumeLayout& out_layout) {
    const std::size_t x1 = token.find('x');
    if (x1 == std::string::npos) {
        return false;
    }
    const std::size_t x2 = token.find('x', x1 + 1U);
    if (x2 == std::string::npos) {
        return false;
    }
    char* end = nullptr;
    const long w = std::strtol(token.c_str(), &end, 10);
    if (end == token.c_str()) {
        return false;
    }
    const long h = std::strtol(token.c_str() + x1 + 1U, &end, 10);
    if (end == token.c_str() + x1 + 1U) {
        return false;
    }
    const long d = std::strtol(token.c_str() + x2 + 1U, &end, 10);
    if (end == token.c_str() + x2 + 1U) {
        return false;
    }
    if (w <= 0 || h <= 0 || d <= 0 || w > static_cast<long>(INT_MAX) || h > static_cast<long>(INT_MAX)
        || d > static_cast<long>(INT_MAX)) {
        return false;
    }
    out_layout.width = static_cast<int>(w);
    out_layout.height = static_cast<int>(h);
    out_layout.depth = static_cast<int>(d);
    return true;
}

[[nodiscard]] bool isGzipFlatPath(const std::string& path) {
    return endsWithBinGz(path);
}

[[nodiscard]] bool readGzipUncompressedSize(const std::string& path, std::uint64_t& out_size) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }
    if (file.tellg() < 4) {
        return false;
    }
    file.seekg(-4, std::ios::end);
    std::uint32_t isize = 0U;
    file.read(reinterpret_cast<char*>(&isize), static_cast<std::streamsize>(sizeof(isize)));
    if (!file) {
        return false;
    }
    out_size = static_cast<std::uint64_t>(isize);
    return true;
}

[[nodiscard]] bool parseDimensionsFromFilename(const std::string& path, vne::image::FlatVolumeLayout& out_layout) {
    const std::size_t slash = path.find_last_of("/\\");
    const std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1U);

    bool found_dims = false;
    for (std::size_t i = 0; i + 2U < name.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
            continue;
        }
        const std::size_t x1 = name.find('x', i);
        if (x1 == std::string::npos) {
            continue;
        }
        const std::size_t x2 = name.find('x', x1 + 1U);
        if (x2 == std::string::npos) {
            continue;
        }
        std::size_t end = x2 + 1U;
        while (end < name.size() && std::isdigit(static_cast<unsigned char>(name[end]))) {
            ++end;
        }
        const std::string token = name.substr(i, end - i);
        vne::image::FlatVolumeLayout candidate = out_layout;
        if (parseDimensions(token, candidate)) {
            out_layout.width = candidate.width;
            out_layout.height = candidate.height;
            out_layout.depth = candidate.depth;
            found_dims = true;
            break;
        }
    }

    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (lower_name.find("uint8") != std::string::npos || lower_name.find("_u8") != std::string::npos) {
        out_layout.pixel_type = vne::image::VolumePixelType::eUint8;
    } else if (lower_name.find("uint16") != std::string::npos || lower_name.find("_u16") != std::string::npos) {
        out_layout.pixel_type = vne::image::VolumePixelType::eUint16;
    } else if (lower_name.find("float32") != std::string::npos || lower_name.find("_f32") != std::string::npos) {
        out_layout.pixel_type = vne::image::VolumePixelType::eFloat32;
    }

    return found_dims;
}

[[nodiscard]] bool inferPixelTypeFromSize(const vne::image::FlatVolumeLayout& layout,
                                          const std::uint64_t uncompressed_bytes,
                                          vne::image::VolumePixelType& inout_type) {
    if (layout.width <= 0 || layout.height <= 0 || layout.depth <= 0) {
        return false;
    }
    const std::uint64_t voxels = static_cast<std::uint64_t>(layout.width) * static_cast<std::uint64_t>(layout.height)
                                 * static_cast<std::uint64_t>(layout.depth);
    if (voxels == 0U) {
        return false;
    }

    const vne::image::VolumePixelType candidates[] = {
        vne::image::VolumePixelType::eUint8,
        vne::image::VolumePixelType::eUint16,
        vne::image::VolumePixelType::eFloat32,
    };
    for (const auto candidate : candidates) {
        const std::size_t bpp = flatBytesPerVoxel(candidate);
        if (bpp == 0U) {
            continue;
        }
        if (voxels * bpp == uncompressed_bytes) {
            inout_type = candidate;
            return true;
        }
    }
    return voxels * flatBytesPerVoxel(inout_type) == uncompressed_bytes;
}

}  // namespace

namespace vne {
namespace image {

bool GzipFlatVolumeLoader::parseLayoutHint(const std::string& hint_format, FlatVolumeLayout& out_layout) {
    if (hint_format.empty()) {
        return false;
    }
    std::string dims = hint_format;
    if (const std::size_t colon = hint_format.find(':'); colon != std::string::npos) {
        const std::string type_token = hint_format.substr(0, colon);
        dims = hint_format.substr(colon + 1U);
        if (type_token == "uint8" || type_token == "uchar") {
            out_layout.pixel_type = VolumePixelType::eUint8;
        } else if (type_token == "uint16" || type_token == "ushort") {
            out_layout.pixel_type = VolumePixelType::eUint16;
        } else if (type_token == "float32" || type_token == "float") {
            out_layout.pixel_type = VolumePixelType::eFloat32;
        } else {
            return false;
        }
    }
    return parseDimensions(dims, out_layout);
}

bool GzipFlatVolumeLoader::isExtensionSupported(const std::string& path) const {
    return isGzipFlatPath(path);
}

bool GzipFlatVolumeLoader::canLoad(const vne::io::LoadRequest& request) const {
    if (request.asset_type != vne::io::AssetType::eVolume) {
        return false;
    }
    if (!request.hint_format.empty() && request.hint_format != "bin-gz" && request.hint_format != "gzip-flat"
        && request.hint_format.find('x') == std::string::npos) {
        return false;
    }
    return isExtensionSupported(request.uri);
}

vne::io::LoadResult<Volume> GzipFlatVolumeLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<Volume> result;
    FlatVolumeLayout layout;
    if (!request.hint_format.empty() && request.hint_format != "bin-gz" && request.hint_format != "gzip-flat") {
        if (!parseLayoutHint(request.hint_format, layout)) {
            result.status = vne::io::Status::make(vne::io::ErrorCode::eInvalidArgument,
                                                  "hint_format must be WxHxD or uint8:WxHxD for gzip flat volumes",
                                                  request.uri,
                                                  "GzipFlatVolumeLoader");
            return result;
        }
        if (!load(request.uri, layout, result.value)) {
            result.status = vne::io::Status::make(vne::io::ErrorCode::eParseError,
                                                  getLastError(),
                                                  request.uri,
                                                  "GzipFlatVolumeLoader");
        } else {
            result.status = vne::io::Status::okStatus();
        }
        return result;
    }

    if (!load(request.uri, result.value)) {
        result.status =
            vne::io::Status::make(vne::io::ErrorCode::eParseError, getLastError(), request.uri, "GzipFlatVolumeLoader");
        return result;
    }
    result.status = vne::io::Status::okStatus();
    return result;
}

bool GzipFlatVolumeLoader::detectLayout(const std::string& path, FlatVolumeLayout& out_layout) const {
    if (!isExtensionSupported(path)) {
        return false;
    }

    FlatVolumeLayout detected = out_layout;
    const bool has_dims = parseDimensionsFromFilename(path, detected);

    std::uint64_t uncompressed_size = 0U;
    if (!readGzipUncompressedSize(path, uncompressed_size) || uncompressed_size == 0U) {
        return has_dims && detected.width > 0 && detected.height > 0 && detected.depth > 0;
    }

    if (has_dims) {
        if (!inferPixelTypeFromSize(detected, uncompressed_size, detected.pixel_type)) {
            return false;
        }
        out_layout = detected;
        return true;
    }

    // No filename dimensions: try cubic-ish factorization for each pixel type.
    for (const auto candidate : {VolumePixelType::eUint8, VolumePixelType::eUint16, VolumePixelType::eFloat32}) {
        const std::size_t bpp = flatBytesPerVoxel(candidate);
        if (bpp == 0U || uncompressed_size % bpp != 0U) {
            continue;
        }
        const std::uint64_t voxels = uncompressed_size / bpp;
        const int side = static_cast<int>(std::cbrt(static_cast<double>(voxels)));
        if (side <= 0) {
            continue;
        }
        if (static_cast<std::uint64_t>(side) * static_cast<std::uint64_t>(side) * static_cast<std::uint64_t>(side)
            != voxels) {
            continue;
        }
        detected.width = side;
        detected.height = side;
        detected.depth = side;
        detected.pixel_type = candidate;
        out_layout = detected;
        return true;
    }

    return false;
}

bool GzipFlatVolumeLoader::load(const std::string& path, Volume& out_volume) {
    FlatVolumeLayout layout;
    if (!detectLayout(path, layout)) {
        last_error_ = "GzipFlatVolumeLoader: failed to infer flat gzip layout (need WxHxD in filename or cubic size)";
        return false;
    }
    return load(path, layout, out_volume);
}

bool GzipFlatVolumeLoader::load(const std::string& path, const FlatVolumeLayout& layout, Volume& out_volume) {
    last_error_.clear();
    out_volume = Volume{};

    if (!isExtensionSupported(path)) {
        last_error_ = "GzipFlatVolumeLoader: expected .bin-gz (gzip-wrapped dense grid; bare .gz is ambiguous)";
        return false;
    }
    if (layout.width <= 0 || layout.height <= 0 || layout.depth <= 0) {
        last_error_ = "GzipFlatVolumeLoader: invalid volume dimensions";
        return false;
    }

    const std::size_t bpp = flatBytesPerVoxel(layout.pixel_type);
    if (bpp == 0U) {
        last_error_ = "GzipFlatVolumeLoader: unsupported pixel type";
        return false;
    }

    vne::io::LoadResult<std::vector<std::uint8_t>> decompressed = vne::io::compression::decompressGzipFile(path);
    if (!decompressed.ok()) {
        last_error_ = decompressed.status.message;
        VNE_LOG_ERROR << "GzipFlatVolumeLoader: " << last_error_ << " ('" << path << "')";
        return false;
    }

    const std::uint64_t expected64 = static_cast<std::uint64_t>(layout.width)
                                     * static_cast<std::uint64_t>(layout.height)
                                     * static_cast<std::uint64_t>(layout.depth) * static_cast<std::uint64_t>(bpp);
    if (expected64 > static_cast<std::uint64_t>(SIZE_MAX)) {
        last_error_ = "GzipFlatVolumeLoader: volume dimensions exceed addressable memory";
        return false;
    }
    const std::size_t expected = static_cast<std::size_t>(expected64);
    if (decompressed.value.size() != expected) {
        last_error_ = "GzipFlatVolumeLoader: size mismatch (got " + std::to_string(decompressed.value.size())
                      + ", expected " + std::to_string(expected) + ")";
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    out_volume.dims[0] = layout.width;
    out_volume.dims[1] = layout.height;
    out_volume.dims[2] = layout.depth;
    out_volume.spacing[0] = layout.spacing[0];
    out_volume.spacing[1] = layout.spacing[1];
    out_volume.spacing[2] = layout.spacing[2];
    out_volume.pixel_type = layout.pixel_type;
    out_volume.components = 1;
    out_volume.data = std::move(decompressed.value);

    VNE_LOG_INFO << "GzipFlatVolumeLoader: loaded " << layout.width << "x" << layout.height << "x" << layout.depth
                 << " from '" << path << "'";
    return true;
}

}  // namespace image
}  // namespace vne
