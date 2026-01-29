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

#include "vertexnova/io/common/binary_io.h"

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
    if (lower == "char" || lower == "int8" || lower == "signed char" || lower == "int8_t") return VolumePixelType::eInt8;
    if (lower == "ushort" || lower == "uint16" || lower == "unsigned short" || lower == "uint16_t") return VolumePixelType::eUint16;
    if (lower == "short" || lower == "int16" || lower == "signed short" || lower == "int16_t") return VolumePixelType::eInt16;
    if (lower == "uint" || lower == "uint32" || lower == "unsigned int" || lower == "uint32_t") return VolumePixelType::eUint32;
    if (lower == "int" || lower == "int32" || lower == "signed int" || lower == "int32_t") return VolumePixelType::eInt32;
    if (lower == "float" || lower == "float32") return VolumePixelType::eFloat32;
    if (lower == "double" || lower == "float64") return VolumePixelType::eFloat64;
    return VolumePixelType::eUnknown;
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

    // Read header until blank line. This is robust for attached-data NRRD.
    std::string header;
    std::streamoff data_offset = 0;
    {
        std::string first_line;
        if (!std::getline(f, first_line)) {
            last_error_ = "NrrdLoader: empty file";
            return false;
        }
        first_line = trim(first_line);
        if (first_line.compare(0, 4, "NRRD") != 0) {
            last_error_ = "NrrdLoader: invalid magic, expected NRRD";
            return false;
        }
        header = first_line + "\n";

        std::string rest;
        std::streamoff off = 0;
        auto st = VNE::IO::BinaryIO::ReadHeaderUntilBlankLine(f, rest, off);
        if (!st) {
            last_error_ = "NrrdLoader: " + st.message;
            return false;
        }
        header += rest;
        data_offset = off;
    }

    // Parse header lines
    std::istringstream hs(header);
    std::string line;

    int dimension = 0;
    int sizes[3] = {0, 0, 0};
    bool have_sizes = false;
    VolumePixelType pixel_type = VolumePixelType::eUnknown;
    std::string encoding;
    std::string data_file;
    float spacings[3] = {1.0f, 1.0f, 1.0f};
    bool has_spacings = false;
    size_t byte_skip = 0;
    int line_skip = 0;
    std::string endian;

    // Skip the first "NRRD..." line, it's already validated.
    std::getline(hs, line);
    while (std::getline(hs, line)) {
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
            // Some files place sizes before dimension. Try parsing as 3 regardless.
            if (!parseSizes(val, sizes, 3)) {
                last_error_ = "NrrdLoader: invalid sizes";
                return false;
            }
            have_sizes = true;
        } else if (key == "type") {
            pixel_type = parseType(val);
            if (pixel_type == VolumePixelType::eUnknown) {
                last_error_ = "NrrdLoader: unsupported type: " + val;
                return false;
            }
        } else if (key == "encoding") {
            std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            encoding = val;
        } else if (key == "data file" || key == "datafile") {
            data_file = trim(val);
        } else if (key == "spacings") {
            const int n = (dimension > 0) ? dimension : 3;
            if (parseSpacings(val, spacings, n)) has_spacings = true;
        } else if (key == "byte skip" || key == "byteskip") {
            byte_skip = static_cast<size_t>(std::stoll(val));
        } else if (key == "line skip" || key == "lineskip") {
            line_skip = std::stoi(val);
        } else if (key == "endian") {
            std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            endian = val;
        }
    }

    if (dimension != 3 || !have_sizes || sizes[0] <= 0 || sizes[1] <= 0 || sizes[2] <= 0) {
        last_error_ = "NrrdLoader: invalid dimension or sizes";
        return false;
    }
    if (pixel_type == VolumePixelType::eUnknown) {
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

    // Load payload: attached or detached.
    if (data_file.empty()) {
        // Attached data starts at `data_offset`.
        f.clear();
        f.seekg(data_offset, std::ios::beg);
        for (int i = 0; i < line_skip && std::getline(f, line); ++i) {}
        if (byte_skip > 0) {
            f.seekg(static_cast<std::streamoff>(byte_skip), std::ios::cur);
        }
        out_volume.data.resize(num_bytes);
        if (!f.read(reinterpret_cast<char*>(out_volume.data.data()), static_cast<std::streamsize>(num_bytes))) {
            last_error_ = "NrrdLoader: failed to read attached data";
            return false;
        }
    } else {
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
    }

    // Endianness fixup if present and multi-byte type.
    if (!endian.empty() && bytesPerVoxel(out_volume.pixel_type) > 1) {
        // Assume host is little endian on most platforms; if you need big-endian hosts, extend this.
        const bool data_is_big = (endian == "big");
        const bool host_is_big = false;
        if (data_is_big != host_is_big) {
            VNE::IO::BinaryIO::ByteSwapBufferInPlace(out_volume.data, bytesPerVoxel(out_volume.pixel_type));
        }
    }

    return true;
}

}  // namespace Image
}  // namespace VNE
