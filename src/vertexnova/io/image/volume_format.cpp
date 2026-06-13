/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/image/volume_format.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {

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

[[nodiscard]] bool pathEndsWithBinGz(const std::string& path) {
    return pathEndsWith(path, ".bin-gz");
}

}  // namespace

namespace vne {
namespace image {

bool VolumePathDescriptor::isRoutable() const {
    if (hasSemanticFormat()) {
        return true;
    }
    if (layout == VolumeVoxelLayout::eSliceStack && archive == VolumeArchive::eTar) {
        return true;
    }
    if (layout == VolumeVoxelLayout::eDenseGrid && compression == VolumeCompression::eGzip
        && archive == VolumeArchive::eNone) {
        return true;
    }
    return false;
}

VolumePathDescriptor parseVolumePath(const std::string& path) {
    VolumePathDescriptor desc;
    if (path.empty()) {
        return desc;
    }

    const std::string lower = lowerString(path);

    // Semantic formats first (self-describing; tar/gzip wrappers not used).
    if (pathEndsWith(lower, ".nrrd") || pathEndsWith(lower, ".nhdr")) {
        desc.semantic = VolumeSemanticFormat::eNrrd;
        return desc;
    }
    if (lower.size() >= 4U
        && (lower.compare(lower.size() - 4U, 4U, ".mhd") == 0 || lower.compare(lower.size() - 4U, 4U, ".mha") == 0)) {
        desc.semantic = VolumeSemanticFormat::eMhd;
        return desc;
    }

    // Order matters: .tar.gz before .tar and .gz.
    if (pathEndsWith(path, ".tar.gz") || pathEndsWith(path, ".tgz")) {
        desc.compression = VolumeCompression::eGzip;
        desc.archive = VolumeArchive::eTar;
        desc.layout = VolumeVoxelLayout::eSliceStack;
        return desc;
    }
    if (pathEndsWith(path, ".tar")) {
        desc.compression = VolumeCompression::eNone;
        desc.archive = VolumeArchive::eTar;
        desc.layout = VolumeVoxelLayout::eSliceStack;
        return desc;
    }

    // Explicit gzip-wrapped dense grid convention (not bare `.gz`).
    if (pathEndsWithBinGz(path)) {
        desc.compression = VolumeCompression::eGzip;
        desc.archive = VolumeArchive::eNone;
        desc.layout = VolumeVoxelLayout::eDenseGrid;
        return desc;
    }

    // Bare `.gz` is ambiguous (gzip(tar) vs gzip(flat)) — do not auto-route.
    return desc;
}

std::string describeVolumePath(const VolumePathDescriptor& desc) {
    if (desc.semantic == VolumeSemanticFormat::eNrrd) {
        return "nrrd (semantic volume format)";
    }
    if (desc.semantic == VolumeSemanticFormat::eMhd) {
        return "mhd/mha (semantic volume format)";
    }

    std::string out;
    if (desc.compression == VolumeCompression::eGzip) {
        out += "gzip";
    } else {
        out += "none";
    }
    out += " + ";
    if (desc.archive == VolumeArchive::eTar) {
        out += "tar";
    } else {
        out += "none";
    }
    out += " + ";
    switch (desc.layout) {
        case VolumeVoxelLayout::eSliceStack:
            out += "slice-stack layout";
            break;
        case VolumeVoxelLayout::eDenseGrid:
            out += "dense-grid layout";
            break;
        case VolumeVoxelLayout::eUnknown:
        default:
            out += "unknown layout";
            break;
    }
    return out;
}

VolumeLoadRoute volumeLoadRouteFromDescriptor(const VolumePathDescriptor& desc) {
    if (desc.semantic == VolumeSemanticFormat::eNrrd) {
        return VolumeLoadRoute::eNrrd;
    }
    if (desc.semantic == VolumeSemanticFormat::eMhd) {
        return VolumeLoadRoute::eMhd;
    }
    if (desc.layout == VolumeVoxelLayout::eSliceStack && desc.archive == VolumeArchive::eTar) {
        return VolumeLoadRoute::eSliceStackArchive;
    }
    if (desc.layout == VolumeVoxelLayout::eDenseGrid && desc.compression == VolumeCompression::eGzip
        && desc.archive == VolumeArchive::eNone) {
        return VolumeLoadRoute::eDenseGridGzip;
    }
    return VolumeLoadRoute::eUnknown;
}

VolumeLoadRoute volumeLoadRouteFromPath(const std::string& path) {
    return volumeLoadRouteFromDescriptor(parseVolumePath(path));
}

const char* volumeLoadRouteName(const VolumeLoadRoute route) {
    switch (route) {
        case VolumeLoadRoute::eSliceStackArchive:
            return "slice-stack-in-tar";
        case VolumeLoadRoute::eDenseGridGzip:
            return "dense-grid-in-gzip";
        case VolumeLoadRoute::eNrrd:
            return "nrrd";
        case VolumeLoadRoute::eMhd:
            return "mhd";
        case VolumeLoadRoute::eUnknown:
        default:
            return "unknown";
    }
}

}  // namespace image
}  // namespace vne
