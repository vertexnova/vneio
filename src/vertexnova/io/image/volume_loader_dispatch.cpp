/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/image/volume_loader_dispatch.h"

#include "vertexnova/io/common/status.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <cctype>

namespace {

CREATE_VNE_LOGGER_CATEGORY("vne.io.image.volume_loader_dispatch")

constexpr std::size_t kTarGzSuffixLen = 7U;  // ".tar.gz"
constexpr std::size_t kBinGzSuffixLen = 7U;  // ".bin-gz"

void mergeFlatHints(vne::image::FlatVolumeLayout& layout, const vne::image::FlatVolumeLayout& hints) {
    if (hints.width > 0) {
        layout.width = hints.width;
    }
    if (hints.height > 0) {
        layout.height = hints.height;
    }
    if (hints.depth > 0) {
        layout.depth = hints.depth;
    }
    if (hints.width > 0 && hints.height > 0 && hints.depth > 0) {
        layout.pixel_type = hints.pixel_type;
    }
    for (int i = 0; i < 3; ++i) {
        if (hints.spacing[i] > 0.0f) {
            layout.spacing[i] = hints.spacing[i];
        }
    }
}

void mergeTarSliceHints(vne::image::TarSliceVolumeLayout& layout, const vne::image::TarSliceVolumeLayout& hints) {
    if (hints.width > 0) {
        layout.width = hints.width;
    }
    if (hints.height > 0) {
        layout.height = hints.height;
    }
    if (hints.depth > 0) {
        layout.depth = hints.depth;
    }
    if (!hints.member_prefix.empty()) {
        layout.member_prefix = hints.member_prefix;
    }
    if (!hints.member_subdirectory.empty()) {
        layout.member_subdirectory = hints.member_subdirectory;
    }
    if (hints.first_slice_index > 0) {
        layout.first_slice_index = hints.first_slice_index;
    }
    if (hints.width > 0 && hints.height > 0 && hints.depth > 0) {
        layout.pixel_type = hints.pixel_type;
        layout.big_endian = hints.big_endian;
    }
    for (int i = 0; i < 3; ++i) {
        if (hints.spacing[i] > 0.0f) {
            layout.spacing[i] = hints.spacing[i];
        }
    }
}

}  // namespace

namespace vne {
namespace image {

bool VolumeLoader::load(const std::string& path, Volume& out_volume, const VolumeLoadHints& hints) {
    last_error_.clear();
    out_volume = Volume{};

    if (path.empty()) {
        last_error_ = "VolumeLoader: empty path";
        return false;
    }

    const VolumePathDescriptor path_desc = parseVolumePath(path);
    const VolumeLoadRoute detected = volumeLoadRouteFromDescriptor(path_desc);
    const VolumeLoadRoute route = hints.format != VolumeLoadRoute::eUnknown ? hints.format : detected;

    if (route == VolumeLoadRoute::eUnknown) {
        std::string lower = path;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (lower.size() >= 3U && lower.compare(lower.size() - 3U, 3U, ".gz") == 0
            && !(lower.size() >= kTarGzSuffixLen
                 && lower.compare(lower.size() - kTarGzSuffixLen, kTarGzSuffixLen, ".tar.gz") == 0)
            && !(lower.size() >= kBinGzSuffixLen
                 && lower.compare(lower.size() - kBinGzSuffixLen, kBinGzSuffixLen, ".bin-gz") == 0)) {
            last_error_ = "VolumeLoader: bare '.gz' is ambiguous (gzip(tar) vs gzip(flat)); use '.tar.gz' or '.bin-gz'";
        } else {
            last_error_ = "VolumeLoader: unrecognized path '" + path
                          + "' — use .nrrd/.mhd, .tar/.tar.gz (slice stack), or .bin-gz (dense grid)";
        }
        return false;
    }

    if (hints.format != VolumeLoadRoute::eUnknown && hints.format != detected) {
        last_error_ = std::string("VolumeLoader: route hint '") + volumeLoadRouteName(hints.format)
                      + "' does not match detected '" + volumeLoadRouteName(detected) + "' ("
                      + describeVolumePath(path_desc) + ") for '" + path + "'";
        VNE_LOG_WARN << last_error_;
        return false;
    }

    switch (route) {
        case VolumeLoadRoute::eDenseGridGzip: {
            if (detected == VolumeLoadRoute::eSliceStackArchive) {
                last_error_ = "VolumeLoader: '" + path + "' is tar + slice-stack layout, not a .bin-gz dense grid";
                VNE_LOG_ERROR << last_error_;
                return false;
            }
            GzipFlatVolumeLoader loader;
            FlatVolumeLayout layout = hints.flat;
            if (layout.width <= 0 || layout.height <= 0 || layout.depth <= 0) {
                if (!loader.detectLayout(path, layout)) {
                    last_error_ = loader.getLastError().empty() ? "VolumeLoader: gzip-flat layout detection failed"
                                                                : loader.getLastError();
                    return false;
                }
                mergeFlatHints(layout, hints.flat);
            }
            if (!loader.load(path, layout, out_volume)) {
                last_error_ = loader.getLastError();
                return false;
            }
            return true;
        }
        case VolumeLoadRoute::eSliceStackArchive: {
            if (detected == VolumeLoadRoute::eDenseGridGzip) {
                last_error_ = "VolumeLoader: '" + path + "' is a .bin-gz dense grid, not tar + slice-stack layout";
                VNE_LOG_ERROR << last_error_;
                return false;
            }
            TarSliceVolumeLoader loader;
            TarSliceVolumeLayout layout = hints.tar_slice;
            const bool has_partial_hints =
                layout.width > 0 || layout.height > 0 || layout.depth > 0 || !layout.member_prefix.empty()
                || !layout.member_subdirectory.empty() || layout.first_slice_index != 1
                || layout.pixel_type != VolumePixelType::eUint16 || !layout.big_endian || layout.spacing[0] != 1.0f
                || layout.spacing[1] != 1.0f || layout.spacing[2] != 1.0f;
            if (!has_partial_hints) {
                if (!loader.load(path, out_volume)) {
                    last_error_ = loader.getLastError();
                    return false;
                }
                return true;
            }
            if (layout.width <= 0 || layout.height <= 0 || layout.depth <= 0) {
                if (!loader.detectLayout(path, layout)) {
                    last_error_ = "VolumeLoader: tar-slice layout detection failed";
                    return false;
                }
            }
            mergeTarSliceHints(layout, hints.tar_slice);
            if (!loader.load(path, layout, out_volume)) {
                last_error_ = loader.getLastError();
                return false;
            }
            return true;
        }
        case VolumeLoadRoute::eNrrd: {
            NrrdLoader loader;
            if (!loader.load(path, out_volume)) {
                last_error_ = loader.getLastError();
                return false;
            }
            return true;
        }
        case VolumeLoadRoute::eMhd: {
            MhdLoader loader;
            if (!loader.load(path, out_volume)) {
                last_error_ = loader.getLastError();
                return false;
            }
            return true;
        }
        case VolumeLoadRoute::eUnknown:
            break;
    }

    last_error_ = "VolumeLoader: unhandled load route";
    return false;
}

vne::io::LoadResult<Volume> VolumeLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<Volume> result;
    VolumeLoadHints hints;
    if (request.hint_format == "tar" || request.hint_format == "tar.gz" || request.hint_format == "tar-slice"
        || request.hint_format == "tgz") {
        hints.format = VolumeLoadRoute::eSliceStackArchive;
    } else if (request.hint_format == "bin-gz" || request.hint_format == "gzip-flat") {
        hints.format = VolumeLoadRoute::eDenseGridGzip;
    } else if (!request.hint_format.empty()) {
        GzipFlatVolumeLoader flat;
        if (flat.parseLayoutHint(request.hint_format, hints.flat)) {
            hints.format = VolumeLoadRoute::eDenseGridGzip;
        }
    }

    if (!load(request.uri, result.value, hints)) {
        result.status =
            vne::io::Status::make(vne::io::ErrorCode::eParseError, getLastError(), request.uri, "VolumeLoader");
        return result;
    }
    result.status = vne::io::Status::okStatus();
    return result;
}

}  // namespace image
}  // namespace vne
