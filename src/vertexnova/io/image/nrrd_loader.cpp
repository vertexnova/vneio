/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/nrrd_loader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace VNE {
namespace Image {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end == std::string::npos ? std::string::npos : end - start + 1);
}

bool parseSizes(const std::string& value, int dims[3], int dimension) {
    std::istringstream iss(value);
    for (int i = 0; i < dimension && iss; ++i) {
        if (!(iss >> dims[i]) || dims[i] <= 0) return false;
    }
    return true;
}

bool parseSpacings(const std::string& value, float spacing[3], int dimension) {
    std::istringstream iss(value);
    for (int i = 0; i < dimension && iss; ++i) {
        if (!(iss >> spacing[i])) return false;
    }
    return true;
}

VolumePixelType parseType(const std::string& t) {
    std::string lower = t;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower == "uchar" || lower == "uint8" || lower == "unsigned char" || lower == "uint8_t") return VolumePixelType::eUint8;
    if (lower == "ushort" || lower == "uint16" || lower == "unsigned short" || lower == "uint16_t") return VolumePixelType::eUint16;
    if (lower == "float") return VolumePixelType::eFloat32;
    return static_cast<VolumePixelType>(-1);
}

std::string dirname(const std::string& path) {
    std::filesystem::path p(path);
    return p.parent_path().string();
}

}  // namespace

bool NrrdLoader::isExtensionSupported(const std::string& path) const {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) return false;
    std::string ext = path.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".nrrd" || ext == ".nhdr";
}

bool NrrdLoader::load(const std::string& path, Volume& out_volume) {
    last_error_.clear();
    out_volume = Volume{};

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        last_error_ = "NrrdLoader: cannot open file: " + path;
        return false;
    }

    std::string line;
    if (!std::getline(f, line)) {
        last_error_ = "NrrdLoader: empty file";
        return false;
    }
    line = trim(line);
    if (line.compare(0, 4, "NRRD") != 0) {
        last_error_ = "NrrdLoader: invalid magic, expected NRRD";
        return false;
    }

    int dimension = 0;
    int sizes[3] = {0, 0, 0};
    VolumePixelType pixel_type = static_cast<VolumePixelType>(-1);
    std::string encoding;
    std::string data_file;
    float spacings[3] = {1.0f, 1.0f, 1.0f};
    bool has_spacings = false;
    size_t byte_skip = 0;
    int line_skip = 0;

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty()) break;
        if (line[0] == '#') continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = trim(line.substr(0, colon));
        std::string val = trim(line.substr(colon + 1));
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (key == "dimension") {
            dimension = std::stoi(val);
            if (dimension != 3) {
                last_error_ = "NrrdLoader: only dimension 3 is supported, got " + std::to_string(dimension);
                return false;
            }
        } else if (key == "sizes") {
            if (!parseSizes(val, sizes, dimension)) {
                last_error_ = "NrrdLoader: invalid sizes";
                return false;
            }
        } else if (key == "type") {
            pixel_type = parseType(val);
            if (static_cast<int>(pixel_type) < 0) {
                last_error_ = "NrrdLoader: unsupported type: " + val;
                return false;
            }
        } else if (key == "encoding") {
            std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            encoding = val;
        } else if (key == "data file" || key == "datafile") {
            data_file = trim(val);
        } else if (key == "spacings") {
            if (parseSpacings(val, spacings, dimension)) has_spacings = true;
        } else if (key == "byte skip" || key == "byteskip") {
            byte_skip = static_cast<size_t>(std::stoll(val));
        } else if (key == "line skip" || key == "lineskip") {
            line_skip = std::stoi(val);
        }
    }

    if (dimension != 3 || sizes[0] <= 0 || sizes[1] <= 0 || sizes[2] <= 0) {
        last_error_ = "NrrdLoader: invalid dimension or sizes";
        return false;
    }
    if (static_cast<int>(pixel_type) < 0) {
        last_error_ = "NrrdLoader: type not set";
        return false;
    }
    if (encoding.empty()) encoding = "raw";
    std::transform(encoding.begin(), encoding.end(), encoding.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (encoding != "raw") {
        last_error_ = "NrrdLoader: only raw encoding is supported, got " + encoding;
        return false;
    }

    out_volume.dims[0] = sizes[0];
    out_volume.dims[1] = sizes[1];
    out_volume.dims[2] = sizes[2];
    out_volume.pixel_type = pixel_type;
    if (has_spacings) {
        out_volume.spacing[0] = spacings[0];
        out_volume.spacing[1] = spacings[1];
        out_volume.spacing[2] = spacings[2];
    }

    size_t num_bytes = out_volume.byteCount();

    if (data_file.empty()) {
        for (int i = 0; i < line_skip && std::getline(f, line); ++i) {}
        if (byte_skip > 0) {
            f.seekg(static_cast<std::streamoff>(byte_skip), std::ios::cur);
        }
        out_volume.data.resize(num_bytes);
        if (!f.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
            last_error_ = "NrrdLoader: failed to read attached data";
            return false;
        }
        return true;
    }

    f.close();

    std::string data_path = data_file;
    if (data_file[0] != '/' && data_file[0] != '\\') {
        std::string base_dir = dirname(path);
        if (!base_dir.empty()) data_path = base_dir + "/" + data_file;
        else data_path = data_file;
    }

    std::ifstream df(data_path, std::ios::binary);
    if (!df) {
        last_error_ = "NrrdLoader: cannot open data file: " + data_path;
        return false;
    }
    for (int i = 0; i < line_skip && std::getline(df, line); ++i) {}
    if (byte_skip > 0) {
        df.seekg(static_cast<std::streamoff>(byte_skip), std::ios::cur);
    }
    out_volume.data.resize(num_bytes);
    if (!df.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
        last_error_ = "NrrdLoader: failed to read detached data";
        return false;
    }
    return true;
}

}  // namespace Image
}  // namespace VNE
