/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/image/tar_slice_volume_loader.h"

#include "vertexnova/io/common/binary_io.h"
#include "vertexnova/io/common/compression.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

CREATE_VNE_LOGGER_CATEGORY("vne.io.image.tar_slice_volume_loader")

constexpr std::size_t kTarBlock = 512U;
constexpr std::size_t kTarNameLen = 100U;

[[nodiscard]] std::string lowerString(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

[[nodiscard]] bool pathEndsWith(const std::string& path, const std::string& suffix) {
    if (path.size() < suffix.size()) {
        return false;
    }
    return lowerString(path.substr(path.size() - suffix.size())) == lowerString(suffix);
}

[[nodiscard]] std::uint64_t parseOctalField(const char* field, const std::size_t len) {
    std::string s(field, len);
    const auto end = s.find('\0');
    if (end != std::string::npos) {
        s.resize(end);
    }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) {
        s.pop_back();
    }
    if (s.empty()) {
        return 0;
    }
    char* endptr = nullptr;
    const unsigned long long v = std::strtoull(s.c_str(), &endptr, 8);
    if (endptr == s.c_str()) {
        return 0;
    }
    return static_cast<std::uint64_t>(v);
}

[[nodiscard]] bool isTarGzPath(const std::string& path) {
    return pathEndsWith(path, ".tar.gz") || pathEndsWith(path, ".tgz");
}

[[nodiscard]] bool isTarPath(const std::string& path) {
    return pathEndsWith(path, ".tar") && !pathEndsWith(path, ".tar.gz");
}

struct TarEntry {
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
};

[[nodiscard]] bool buildTarIndex(const std::uint8_t* data,
                                 const std::size_t size,
                                 std::unordered_map<std::string, TarEntry>& out_index) {
    out_index.clear();
    std::size_t offset = 0;
    while (offset + kTarBlock <= size) {
        const std::uint8_t* hdr = data + offset;
        bool all_zero = true;
        for (std::size_t i = 0; i < kTarBlock; ++i) {
            if (hdr[i] != 0) {
                all_zero = false;
                break;
            }
        }
        if (all_zero) {
            break;
        }

        char name[kTarNameLen + 1U];
        std::memcpy(name, hdr, kTarNameLen);
        name[kTarNameLen] = '\0';
        const std::string entry_name(name);

        const char typeflag = static_cast<char>(hdr[156]);
        const std::uint64_t file_size = parseOctalField(reinterpret_cast<const char*>(hdr + 124), 12);
        offset += kTarBlock;

        if ((typeflag == '0' || typeflag == '\0') && file_size > 0) {
            TarEntry entry;
            entry.offset = offset;
            entry.size = file_size;
            out_index[entry_name] = entry;
        }

        const std::size_t remaining = size - offset;
        if (file_size > static_cast<std::uint64_t>(remaining)) {
            return false;
        }
        const std::uint64_t data_blocks = (file_size + kTarBlock - 1U) / kTarBlock;
        const std::uint64_t data_bytes = data_blocks * kTarBlock;
        if (data_bytes > static_cast<std::uint64_t>(remaining)) {
            return false;
        }
        offset += static_cast<std::size_t>(data_bytes);
    }
    return true;
}

[[nodiscard]] bool loadTarBytes(const std::string& path, std::vector<std::uint8_t>& out_tar) {
    if (isTarGzPath(path)) {
        vne::io::LoadResult<std::vector<std::uint8_t>> gz = vne::io::compression::decompressGzipFile(path);
        if (!gz.ok()) {
            VNE_LOG_ERROR << "TarSliceVolumeLoader: gzip read failed for '" << path << "': " << gz.status.message;
            return false;
        }
        out_tar = std::move(gz.value);
        return true;
    }
    if (isTarPath(path)) {
        const vne::io::Status st = vne::io::binaryio::readFile(path, out_tar);
        if (!st.ok()) {
            VNE_LOG_ERROR << "TarSliceVolumeLoader: tar read failed for '" << path << "': " << st.message;
            return false;
        }
        return true;
    }
    return false;
}

[[nodiscard]] std::string sliceMemberName(const vne::image::TarSliceVolumeLayout& layout, const int slice_index) {
    return layout.member_subdirectory + layout.member_prefix + std::to_string(slice_index);
}

struct SliceGroup {
    std::string prefix;
    std::string subdir;
    int first_index = 0;
    int last_index = 0;
    std::uint64_t slice_bytes = 0;
};

[[nodiscard]] bool parseSliceMember(const std::string& name,
                                    std::string& out_prefix,
                                    std::string& out_subdir,
                                    int& out_index) {
    if (name.empty()) {
        return false;
    }
    const std::size_t slash = name.find_last_of('/');
    if (slash != std::string::npos) {
        out_subdir = name.substr(0, slash + 1U);
        const std::string tail = name.substr(slash + 1U);
        const std::size_t digit_start = tail.find_first_of("0123456789");
        if (digit_start == std::string::npos) {
            return false;
        }
        out_prefix = tail.substr(0, digit_start);
        out_index = std::atoi(tail.c_str() + digit_start);
        return out_index > 0;
    }
    const std::size_t digit_start = name.find_first_of("0123456789");
    if (digit_start == std::string::npos || digit_start == 0) {
        return false;
    }
    out_subdir.clear();
    out_prefix = name.substr(0, digit_start);
    out_index = std::atoi(name.c_str() + digit_start);
    return out_index > 0;
}

[[nodiscard]] bool inferSquareDimensions(const std::uint64_t slice_bytes,
                                         const std::size_t bytes_per_voxel,
                                         int& out_width,
                                         int& out_height) {
    if (bytes_per_voxel == 0U || slice_bytes % bytes_per_voxel != 0U) {
        return false;
    }
    const std::uint64_t voxel_count = slice_bytes / bytes_per_voxel;
    const int side = static_cast<int>(std::sqrt(static_cast<double>(voxel_count)));
    if (side <= 0 || static_cast<std::uint64_t>(side) * static_cast<std::uint64_t>(side) != voxel_count) {
        return false;
    }
    out_width = side;
    out_height = side;
    return true;
}

}  // namespace

namespace vne {
namespace image {

bool TarSliceVolumeLoader::isExtensionSupported(const std::string& path) const {
    return isTarPath(path) || isTarGzPath(path);
}

bool TarSliceVolumeLoader::canLoad(const vne::io::LoadRequest& request) const {
    if (request.asset_type != vne::io::AssetType::eVolume) {
        return false;
    }
    if (!request.hint_format.empty() && request.hint_format != "tar" && request.hint_format != "tar.gz"
        && request.hint_format != "tar-slice" && request.hint_format != "tgz") {
        return false;
    }
    return isExtensionSupported(request.uri);
}

bool TarSliceVolumeLoader::detectLayout(const std::string& path, TarSliceVolumeLayout& inout_layout) const {
    std::vector<std::uint8_t> tar_bytes;
    if (!loadTarBytes(path, tar_bytes)) {
        return false;
    }

    std::unordered_map<std::string, TarEntry> index;
    if (!buildTarIndex(tar_bytes.data(), tar_bytes.size(), index)) {
        return false;
    }

    std::unordered_map<std::string, SliceGroup> groups;
    for (const auto& [name, entry] : index) {
        std::string prefix;
        std::string subdir;
        int index_num = 0;
        if (!parseSliceMember(name, prefix, subdir, index_num)) {
            continue;
        }
        const std::string key = subdir + "|" + prefix;
        SliceGroup& group = groups[key];
        if (group.slice_bytes == 0) {
            group.prefix = prefix;
            group.subdir = subdir;
            group.first_index = index_num;
            group.last_index = index_num;
            group.slice_bytes = entry.size;
        } else {
            group.first_index = std::min(group.first_index, index_num);
            group.last_index = std::max(group.last_index, index_num);
            if (group.slice_bytes != entry.size) {
                group.slice_bytes = 0;
            }
        }
    }

    const SliceGroup* best = nullptr;
    int best_depth = 0;
    for (const auto& [_, group] : groups) {
        if (group.slice_bytes == 0) {
            continue;
        }
        const int depth = group.last_index - group.first_index + 1;
        if (depth > best_depth) {
            best_depth = depth;
            best = &group;
        }
    }
    if (best == nullptr || best_depth <= 0) {
        return false;
    }

    if (inout_layout.member_prefix.empty()) {
        inout_layout.member_prefix = best->prefix;
    }
    if (inout_layout.member_subdirectory.empty()) {
        inout_layout.member_subdirectory = best->subdir;
    }
    if (inout_layout.first_slice_index <= 0) {
        inout_layout.first_slice_index = best->first_index;
    }
    if (inout_layout.depth <= 0) {
        inout_layout.depth = best_depth;
    }

    const auto bpp = static_cast<std::size_t>(bytesPerVoxel(inout_layout.pixel_type));
    if (inout_layout.width <= 0 || inout_layout.height <= 0) {
        if (!inferSquareDimensions(best->slice_bytes, bpp, inout_layout.width, inout_layout.height)) {
            return false;
        }
    }
    return inout_layout.width > 0 && inout_layout.height > 0 && inout_layout.depth > 0;
}

vne::io::LoadResult<Volume> TarSliceVolumeLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<Volume> result;
    if (!load(request.uri, result.value)) {
        result.status =
            vne::io::Status::make(vne::io::ErrorCode::eParseError, getLastError(), request.uri, "TarSliceVolumeLoader");
        return result;
    }
    result.status = vne::io::Status::okStatus();
    return result;
}

bool TarSliceVolumeLoader::load(const std::string& path, Volume& out_volume) {
    TarSliceVolumeLayout layout;
    if (!detectLayout(path, layout)) {
        last_error_ = "TarSliceVolumeLoader: failed to infer slice layout";
        return false;
    }
    return load(path, layout, out_volume);
}

bool TarSliceVolumeLoader::load(const std::string& path, const TarSliceVolumeLayout& layout_in, Volume& out_volume) {
    last_error_.clear();
    out_volume = Volume{};

    if (!isExtensionSupported(path)) {
        last_error_ = "TarSliceVolumeLoader: expected tar-slice archive (.tar or .tar.gz/.tgz, not gzip-flat .bin-gz)";
        return false;
    }

    TarSliceVolumeLayout layout = layout_in;
    if (layout.width <= 0 || layout.height <= 0 || layout.depth <= 0) {
        if (!detectLayout(path, layout)) {
            last_error_ = "TarSliceVolumeLoader: incomplete layout and auto-detect failed";
            return false;
        }
    }

    std::vector<std::uint8_t> tar_bytes;
    if (!loadTarBytes(path, tar_bytes)) {
        last_error_ = "TarSliceVolumeLoader: failed to read archive";
        VNE_LOG_ERROR << last_error_ << " ('" << path << "')";
        return false;
    }

    std::unordered_map<std::string, TarEntry> index;
    if (!buildTarIndex(tar_bytes.data(), tar_bytes.size(), index)) {
        last_error_ = "TarSliceVolumeLoader: tar index failed";
        return false;
    }

    const int width = layout.width;
    const int height = layout.height;
    const int depth = layout.depth;
    const auto bytes_per_voxel = static_cast<std::size_t>(bytesPerVoxel(layout.pixel_type));
    const std::size_t slice_bytes =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * bytes_per_voxel;
    const std::size_t voxel_count =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(depth);
    const std::size_t total_bytes = voxel_count * bytes_per_voxel;

    std::vector<std::uint8_t> volume_data(total_bytes, 0);

    for (int z = 0; z < depth; ++z) {
        const int slice_index = layout.first_slice_index + z;
        const std::string member = sliceMemberName(layout, slice_index);
        const auto it = index.find(member);
        if (it == index.end()) {
            last_error_ = "TarSliceVolumeLoader: missing slice '" + member + "'";
            VNE_LOG_ERROR << last_error_;
            return false;
        }
        if (it->second.size < slice_bytes) {
            last_error_ = "TarSliceVolumeLoader: slice '" + member + "' too small";
            return false;
        }
        if (it->second.offset + slice_bytes > tar_bytes.size()) {
            last_error_ = "TarSliceVolumeLoader: slice '" + member + "' out of range";
            return false;
        }

        const std::size_t dst_slice_off = static_cast<std::size_t>(z) * slice_bytes;
        std::memcpy(volume_data.data() + dst_slice_off, tar_bytes.data() + it->second.offset, slice_bytes);
        if (layout.big_endian && bytes_per_voxel == 2U) {
            for (std::size_t i = 0; i < slice_bytes; i += 2U) {
                std::swap(volume_data[dst_slice_off + i], volume_data[dst_slice_off + i + 1U]);
            }
        }
    }

    out_volume.dims[0] = width;
    out_volume.dims[1] = height;
    out_volume.dims[2] = depth;
    out_volume.spacing[0] = layout.spacing[0];
    out_volume.spacing[1] = layout.spacing[1];
    out_volume.spacing[2] = layout.spacing[2];
    out_volume.pixel_type = layout.pixel_type;
    out_volume.components = 1;
    out_volume.data = std::move(volume_data);

    VNE_LOG_INFO << "TarSliceVolumeLoader: loaded " << width << "x" << height << "x" << depth << " from '" << path
                 << "'";
    return true;
}

}  // namespace image
}  // namespace vne
