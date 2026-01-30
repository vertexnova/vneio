/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/mhd_loader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <filesystem>

#include "vertexnova/io/common/binary_io.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"

namespace vne {
namespace Image {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end == std::string::npos ? std::string::npos : end - start + 1);
}

bool parseDimSize(const std::string& value, int dims[3], int ndims) {
    std::istringstream iss(value);
    for (int i = 0; i < ndims && iss; ++i) {
        if (!(iss >> dims[i]) || dims[i] <= 0) {
            return false;
        }
    }
    return true;
}

bool parseElementSpacing(const std::string& value, float spacing[3], int ndims) {
    std::istringstream iss(value);
    for (int i = 0; i < ndims && iss; ++i) {
        if (!(iss >> spacing[i])) {
            return false;
        }
    }
    return true;
}

VolumePixelType parseElementType(const std::string& t) {
    std::string upper = t;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    if (upper == "MET_UCHAR") {
        return VolumePixelType::eUint8;
    }
    if (upper == "MET_CHAR") {
        return VolumePixelType::eInt8;
    }
    if (upper == "MET_USHORT") {
        return VolumePixelType::eUint16;
    }
    if (upper == "MET_SHORT") {
        return VolumePixelType::eInt16;
    }
    if (upper == "MET_UINT") {
        return VolumePixelType::eUint32;
    }
    if (upper == "MET_INT") {
        return VolumePixelType::eInt32;
    }
    if (upper == "MET_FLOAT") {
        return VolumePixelType::eFloat32;
    }
    if (upper == "MET_DOUBLE") {
        return VolumePixelType::eFloat64;
    }
    return VolumePixelType::eUnknown;
}

std::string dirname(const std::string& path) {
    std::filesystem::path p(path);
    return p.parent_path().string();
}

}  // namespace

bool MhdLoader::canLoad(const vne::io::LoadRequest& request) const {
    if (request.asset_type != vne::io::AssetType::eVolume) {
        return false;
    }
    return isExtensionSupported(request.uri);
}

vne::io::LoadResult<vne::Image::Volume> MhdLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<vne::Image::Volume> result;
    if (!load(request.uri, result.value)) {
        result.status =
            vne::io::Status::make(vne::io::ErrorCode::eParseError, getLastError(), request.uri, "MhdLoader");
        return result;
    }
    result.status = vne::io::Status::okStatus();
    return result;
}

bool MhdLoader::isExtensionSupported(const std::string& path) const {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return false;
    }
    std::string ext = path.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".mhd" || ext == ".mha";
}

bool MhdLoader::load(const std::string& path, Volume& out_volume) {
    last_error_.clear();
    out_volume = Volume{};

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        last_error_ = "MhdLoader: cannot open file: " + path;
        return false;
    }

    int ndims = 0;
    int dims[3] = {0, 0, 0};
    VolumePixelType pixel_type = VolumePixelType::eUnknown;
    float spacing[3] = {1.0f, 1.0f, 1.0f};
    std::string element_data_file;
    bool msb = false;
    std::string line;

    // For .mha (ElementDataFile = LOCAL), binary starts right after the header blank line.
    // We therefore parse the header first (terminated by a blank line), then use the recorded offset.
    std::streamoff data_start_offset = -1;

    std::string header;
    {
        std::streamoff off = 0;
        auto st = vne::io::BinaryIO::ReadHeaderUntilBlankLine(f, header, off);
        if (!st) {
            last_error_ = "MhdLoader: " + st.message;
            return false;
        }
        data_start_offset = off;
    }

    std::istringstream hs(header);

    while (std::getline(hs, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });

        if (key == "NDIMS") {
            ndims = std::stoi(val);
            if (ndims != 3) {
                last_error_ = "MhdLoader: only NDims 3 is supported, got " + std::to_string(ndims);
                return false;
            }
        } else if (key == "DIMSIZE") {
            // Some files place DimSize before NDims; parse as 3 regardless.
            if (!parseDimSize(val, dims, 3)) {
                last_error_ = "MhdLoader: invalid DimSize";
                return false;
            }
        } else if (key == "ELEMENTTYPE") {
            pixel_type = parseElementType(val);
            if (pixel_type == VolumePixelType::eUnknown) {
                last_error_ = "MhdLoader: unsupported ElementType: " + val;
                return false;
            }
        } else if (key == "ELEMENTSPACING") {
            parseElementSpacing(val, spacing, (ndims > 0) ? ndims : 3);
        } else if (key == "ELEMENTDATAFILE") {
            element_data_file = trim(val);
        } else if (key == "ELEMENTBYTEORDERMSB") {
            msb = (val.find("TRUE") != std::string::npos || val.find("True") != std::string::npos || val == "1");
        }
    }

    if (ndims != 3 || dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0) {
        last_error_ = "MhdLoader: invalid NDims or DimSize";
        return false;
    }
    if (pixel_type == VolumePixelType::eUnknown) {
        last_error_ = "MhdLoader: ElementType not set";
        return false;
    }

    out_volume.dims[0] = dims[0];
    out_volume.dims[1] = dims[1];
    out_volume.dims[2] = dims[2];
    out_volume.pixel_type = pixel_type;
    out_volume.spacing[0] = spacing[0];
    out_volume.spacing[1] = spacing[1];
    out_volume.spacing[2] = spacing[2];

    size_t num_bytes = out_volume.byteCount();

    std::string element_data_file_upper = element_data_file;
    std::transform(element_data_file_upper.begin(),
                   element_data_file_upper.end(),
                   element_data_file_upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (element_data_file_upper == "LOCAL" || element_data_file.empty()) {
        if (data_start_offset < 0) {
            last_error_ = "MhdLoader: ElementDataFile LOCAL but could not determine data start";
            return false;
        }
        f.clear();
        f.seekg(data_start_offset, std::ios::beg);
        out_volume.data.resize(num_bytes);
        if (!f.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
            last_error_ = "MhdLoader: failed to read inline data (ElementDataFile = LOCAL)";
            return false;
        }
        return true;
    }

    f.close();

    std::string data_path = element_data_file;
    std::string base_dir = dirname(path);
    if (!base_dir.empty() && !element_data_file.empty()) {
        data_path = base_dir + "/" + element_data_file;
    }

    std::ifstream df(data_path, std::ios::binary);
    if (!df) {
        last_error_ = "MhdLoader: cannot open data file: " + data_path;
        return false;
    }
    out_volume.data.resize(num_bytes);
    if (!df.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
        last_error_ = "MhdLoader: failed to read data file";
        return false;
    }
    if (msb && bytesPerVoxel(pixel_type) > 1) {
        int b = bytesPerVoxel(pixel_type);
        size_t n = out_volume.voxelCount();
        for (size_t i = 0; i < n; ++i) {
            uint8_t* p = out_volume.data.data() + i * static_cast<size_t>(b);
            for (int j = 0; j < b / 2; ++j) {
                std::swap(p[j], p[b - 1 - j]);
            }
        }
    }
    return true;
}

}  // namespace Image
}  // namespace vne
