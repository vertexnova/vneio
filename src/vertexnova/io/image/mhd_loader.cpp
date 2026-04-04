/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/common/binary_io.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/logging/logging.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace {

CREATE_VNE_LOGGER_CATEGORY("vne.io.image.mhd_loader");

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

bool parseThreeFloats(const std::string& value, float out[3]) {
    std::istringstream iss(value);
    for (int i = 0; i < 3; ++i) {
        if (!(iss >> out[i]) || !std::isfinite(out[i])) {
            return false;
        }
    }
    return true;
}

bool parseNineFloats(const std::string& value, float m[9]) {
    std::istringstream iss(value);
    for (int i = 0; i < 9; ++i) {
        if (!(iss >> m[i]) || !std::isfinite(m[i])) {
            return false;
        }
    }
    return true;
}

void byteSwapVolumeData(vne::image::Volume& vol, vne::image::VolumePixelType pixel_type, bool msb) {
    if (!msb || vne::image::bytesPerVoxel(pixel_type) <= 1) {
        return;
    }
    int b = vne::image::bytesPerVoxel(pixel_type);
    size_t n = vol.voxelCount();
    for (size_t i = 0; i < n; ++i) {
        uint8_t* p = vol.data.data() + i * static_cast<size_t>(b);
        for (int j = 0; j < b / 2; ++j) {
            std::swap(p[j], p[b - 1 - j]);
        }
    }
}

float determinant3(const float d[9]) {
    return d[0] * (d[4] * d[8] - d[5] * d[7]) - d[1] * (d[3] * d[8] - d[5] * d[6]) + d[2] * (d[3] * d[7] - d[4] * d[6]);
}

/** Normalize rows of 3×3 row-major matrix in place; returns false if any row degenerate or |det| tiny. */
bool normalizeDirectionRows(float d[9], float min_det) {
    for (int row = 0; row < 3; ++row) {
        int b = row * 3;
        float x = d[b + 0];
        float y = d[b + 1];
        float z = d[b + 2];
        float len = std::sqrt(x * x + y * y + z * z);
        if (!std::isfinite(len) || len < 1e-20f) {
            return false;
        }
        d[b + 0] = x / len;
        d[b + 1] = y / len;
        d[b + 2] = z / len;
    }
    float det = determinant3(d);
    return std::isfinite(det) && std::fabs(det) >= min_det;
}

vne::image::VolumePixelType parseElementType(const std::string& t) {
    std::string upper = t;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    using vne::image::VolumePixelType;
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

namespace vne {
namespace image {

bool MhdLoader::canLoad(const vne::io::LoadRequest& request) const {
    if (request.asset_type != vne::io::AssetType::eVolume) {
        return false;
    }
    return isExtensionSupported(request.uri);
}

vne::io::LoadResult<vne::image::Volume> MhdLoader::loadVolume(const vne::io::LoadRequest& request) {
    vne::io::LoadResult<vne::image::Volume> result;
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
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    int ndims = 0;
    int dims[3] = {0, 0, 0};
    VolumePixelType pixel_type = VolumePixelType::eUnknown;
    float spacing[3] = {1.0f, 1.0f, 1.0f};
    std::string element_data_file;
    bool msb = false;
    std::string line;

    float origin_from_position[3] = {0, 0, 0};
    float origin_from_offset[3] = {0, 0, 0};
    bool have_position = false;
    bool have_offset = false;
    float transform_matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    bool have_transform = false;
    int element_channels = 1;
    bool have_element_channels = false;

    std::streamoff data_start_offset = -1;

    std::string header;
    {
        std::streamoff off = 0;
        auto st = vne::io::binaryio::readHeaderUntilBlankLine(f, header, off);
        if (!st) {
            last_error_ = "MhdLoader: " + st.message;
            VNE_LOG_ERROR << last_error_;
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
                VNE_LOG_ERROR << last_error_;
                return false;
            }
        } else if (key == "DIMSIZE") {
            if (!parseDimSize(val, dims, 3)) {
                last_error_ = "MhdLoader: invalid DimSize";
                VNE_LOG_ERROR << last_error_;
                return false;
            }
        } else if (key == "ELEMENTTYPE") {
            pixel_type = parseElementType(val);
            if (pixel_type == VolumePixelType::eUnknown) {
                last_error_ = "MhdLoader: unsupported ElementType: " + val;
                VNE_LOG_ERROR << last_error_;
                return false;
            }
        } else if (key == "ELEMENTSPACING") {
            parseElementSpacing(val, spacing, (ndims > 0) ? ndims : 3);
        } else if (key == "ELEMENTDATAFILE") {
            element_data_file = trim(val);
        } else if (key == "ELEMENTBYTEORDERMSB") {
            msb = (val.find("TRUE") != std::string::npos || val.find("True") != std::string::npos || val == "1");
            VNE_LOG_DEBUG << "MhdLoader: ElementByteOrderMSB=" << (msb ? "true" : "false");
        } else if (key == "POSITION") {
            if (parseThreeFloats(val, origin_from_position)) {
                have_position = true;
            }
        } else if (key == "OFFSET") {
            if (parseThreeFloats(val, origin_from_offset)) {
                have_offset = true;
            }
        } else if (key == "TRANSFORMMATRIX") {
            if (parseNineFloats(val, transform_matrix)) {
                have_transform = true;
            }
        } else if (key == "ELEMENTNUMBEROFCHANNELS") {
            element_channels = std::stoi(val);
            have_element_channels = true;
        }
    }

    if (ndims != 3 || dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0) {
        last_error_ = "MhdLoader: invalid NDims or DimSize";
        VNE_LOG_ERROR << last_error_;
        return false;
    }
    if (pixel_type == VolumePixelType::eUnknown) {
        last_error_ = "MhdLoader: ElementType not set";
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    if (have_element_channels && element_channels != 1) {
        last_error_ = "MhdLoader: only scalar volumes supported (ElementNumberOfChannels="
                      + std::to_string(element_channels) + ")";
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    out_volume.dims[0] = dims[0];
    out_volume.dims[1] = dims[1];
    out_volume.dims[2] = dims[2];
    out_volume.pixel_type = pixel_type;
    out_volume.components = 1;
    out_volume.spacing[0] = spacing[0];
    out_volume.spacing[1] = spacing[1];
    out_volume.spacing[2] = spacing[2];

    if (have_position) {
        out_volume.origin[0] = origin_from_position[0];
        out_volume.origin[1] = origin_from_position[1];
        out_volume.origin[2] = origin_from_position[2];
        VNE_LOG_INFO << "MhdLoader: using Position for origin";
    } else if (have_offset) {
        out_volume.origin[0] = origin_from_offset[0];
        out_volume.origin[1] = origin_from_offset[1];
        out_volume.origin[2] = origin_from_offset[2];
        VNE_LOG_INFO << "MhdLoader: using Offset for origin (Position absent)";
    } else {
        VNE_LOG_INFO << "MhdLoader: no Position/Offset; origin (0,0,0)";
    }

    if (have_position && have_offset) {
        float d0 = std::fabs(origin_from_position[0] - origin_from_offset[0]);
        float d1 = std::fabs(origin_from_position[1] - origin_from_offset[1]);
        float d2 = std::fabs(origin_from_position[2] - origin_from_offset[2]);
        if (d0 > 1e-4f || d1 > 1e-4f || d2 > 1e-4f) {
            VNE_LOG_WARN << "MhdLoader: Position and Offset differ; using Position";
        }
    }

    static const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
    std::memcpy(out_volume.direction, identity, sizeof(identity));
    if (have_transform) {
        float tm[9];
        std::memcpy(tm, transform_matrix, sizeof(tm));
        if (normalizeDirectionRows(tm, 1e-4f)) {
            std::memcpy(out_volume.direction, tm, sizeof(tm));
            VNE_LOG_INFO << "MhdLoader: loaded TransformMatrix (rows normalized)";
        } else {
            VNE_LOG_WARN << "MhdLoader: TransformMatrix degenerate or singular after normalization; using identity";
        }
    } else {
        VNE_LOG_INFO << "MhdLoader: no TransformMatrix; direction identity";
    }

    size_t num_bytes = out_volume.byteCount();

    std::string element_data_file_upper = element_data_file;
    std::transform(element_data_file_upper.begin(),
                   element_data_file_upper.end(),
                   element_data_file_upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (element_data_file_upper == "LOCAL" || element_data_file.empty()) {
        if (data_start_offset < 0) {
            last_error_ = "MhdLoader: ElementDataFile LOCAL but could not determine data start";
            VNE_LOG_ERROR << last_error_;
            return false;
        }
        f.clear();
        f.seekg(data_start_offset, std::ios::beg);
        out_volume.data.resize(num_bytes);
        if (!f.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
            last_error_ = "MhdLoader: failed to read inline data (ElementDataFile = LOCAL)";
            VNE_LOG_ERROR << last_error_;
            return false;
        }
        if (f.gcount() != static_cast<std::streamsize>(num_bytes)) {
            last_error_ = "MhdLoader: short read inline data (expected " + std::to_string(num_bytes) + " bytes)";
            VNE_LOG_ERROR << last_error_;
            return false;
        }
        byteSwapVolumeData(out_volume, pixel_type, msb);
        f.clear();
        f.seekg(0, std::ios::end);
        const auto fend = f.tellg();
        if (fend != static_cast<std::streamoff>(-1) && data_start_offset >= 0) {
            const auto expected_end =
                static_cast<std::streamoff>(data_start_offset) + static_cast<std::streamoff>(num_bytes);
            if (fend > expected_end) {
                VNE_LOG_WARN << "MhdLoader: " << (fend - expected_end) << " trailing byte(s) after inline payload in \""
                             << path << "\"";
            }
        }
        if (!out_volume.hasExactBufferSize()) {
            last_error_ = "MhdLoader: internal buffer size error";
            VNE_LOG_ERROR << last_error_;
            return false;
        }
        VNE_LOG_INFO << "MhdLoader: loaded \"" << path << "\" (inline) dims=" << out_volume.dims[0] << "x"
                     << out_volume.dims[1] << "x" << out_volume.dims[2];
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
        VNE_LOG_ERROR << last_error_;
        return false;
    }
    out_volume.data.resize(num_bytes);
    if (!df.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
        last_error_ = "MhdLoader: failed to read data file";
        VNE_LOG_ERROR << last_error_;
        return false;
    }
    if (df.gcount() != static_cast<std::streamsize>(num_bytes)) {
        last_error_ = "MhdLoader: short read data file (expected " + std::to_string(num_bytes) + " bytes)";
        VNE_LOG_ERROR << last_error_;
        return false;
    }
    byteSwapVolumeData(out_volume, pixel_type, msb);

    df.clear();
    df.seekg(0, std::ios::end);
    const auto dend = df.tellg();
    const auto expected_sz = static_cast<std::streamoff>(num_bytes);
    if (dend != static_cast<std::streamoff>(-1) && dend > expected_sz) {
        VNE_LOG_WARN << "MhdLoader: data file \"" << data_path << "\" has " << (dend - expected_sz)
                     << " extra byte(s) after payload";
    }

    if (!out_volume.hasExactBufferSize()) {
        last_error_ = "MhdLoader: internal buffer size error";
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    VNE_LOG_INFO << "MhdLoader: loaded \"" << path << "\" dims=" << out_volume.dims[0] << "x" << out_volume.dims[1]
                 << "x" << out_volume.dims[2] << " data=\"" << data_path << "\"";

    return true;
}

}  // namespace image
}  // namespace vne
