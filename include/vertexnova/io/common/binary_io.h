#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Small IO helpers (no third-party deps).
 * Read header until blank line; read/write full buffers; byte swap.
 * ----------------------------------------------------------------------
 */

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "vertexnova/io/common/status.h"

namespace VNE {
namespace IO {
namespace BinaryIO {

inline Status ReadFile(const std::string& path, std::vector<uint8_t>& out) {
    out.clear();
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return Status::make(ErrorCode::eFileOpenFailed, "Cannot open file", path, "BinaryIO");
    }
    f.seekg(0, std::ios::end);
    const std::streamoff size = f.tellg();
    if (size < 0) {
        return Status::make(ErrorCode::eFileReadFailed, "Failed to determine file size", path, "BinaryIO");
    }
    f.seekg(0, std::ios::beg);
    out.resize(static_cast<size_t>(size));
    if (!out.empty() && !f.read(reinterpret_cast<char*>(out.data()), size)) {
        out.clear();
        return Status::make(ErrorCode::eFileReadFailed, "Failed to read file", path, "BinaryIO");
    }
    return Status::okStatus();
}

inline Status WriteFile(const std::string& path, const void* data, size_t size) {
    if (size > 0 && data == nullptr) {
        return Status::make(ErrorCode::eInvalidArgument, "WriteFile: data is null", path, "BinaryIO");
    }
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        return Status::make(ErrorCode::eFileOpenFailed, "Cannot open file for writing", path, "BinaryIO");
    }
    if (size > 0) {
        f.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        if (!f) {
            return Status::make(ErrorCode::eFileWriteFailed, "Failed to write file", path, "BinaryIO");
        }
    }
    return Status::okStatus();
}

/**
 * Reads a text header terminated by the first blank line.
 * header_text includes all header bytes up to and including the blank line.
 * data_offset is the absolute offset in file where the binary payload starts.
 */
inline Status ReadHeaderUntilBlankLine(std::ifstream& f, std::string& header_text, std::streamoff& data_offset) {
    header_text.clear();
    data_offset = 0;
    if (!f) {
        return Status::make(ErrorCode::eFileReadFailed,
                             "ReadHeaderUntilBlankLine: invalid stream",
                             {},
                             "BinaryIO");
    }
    std::string line;
    while (std::getline(f, line)) {
        header_text += line;
        header_text += "\n";
        if (line.empty()) {
            data_offset = f.tellg();
            if (data_offset < 0) {
                data_offset = 0;
            }
            return Status::okStatus();
        }
    }
    return Status::make(ErrorCode::eDataTruncated, "Header not terminated with blank line", {}, "BinaryIO");
}

inline void ByteSwapInPlace(uint8_t* bytes, int elem_size) {
    for (int j = 0; j < elem_size / 2; ++j) {
        const int k = elem_size - 1 - j;
        std::swap(bytes[j], bytes[k]);
    }
}

inline void ByteSwapBufferInPlace(std::vector<uint8_t>& buf, int elem_size) {
    if (elem_size <= 1) {
        return;
    }
    const size_t n = buf.size() / static_cast<size_t>(elem_size);
    for (size_t i = 0; i < n; ++i) {
        ByteSwapInPlace(buf.data() + i * static_cast<size_t>(elem_size), elem_size);
    }
}

}  // namespace BinaryIO
}  // namespace IO
}  // namespace VNE
