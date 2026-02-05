#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/common/status.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace vne {
namespace io {
namespace binaryio {

/**
 * @file binary_io.h
 * @brief Small IO helpers: read/write full buffers, read header until blank line, byte swap.
 */

/**
 * @brief Read entire file into a byte vector.
 * @param path File path.
 * @param out Output vector (cleared and filled).
 * @return Status (eOk on success).
 */
[[nodiscard]] inline Status readFile(const std::string& path, std::vector<uint8_t>& out) {
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

/**
 * @brief Write a buffer to a file.
 * @param path File path.
 * @param data Pointer to data (may be nullptr only if size is 0).
 * @param size Number of bytes to write.
 * @return Status (eOk on success).
 */
[[nodiscard]] inline Status writeFile(const std::string& path, const void* data, size_t size) {
    if (size > 0 && data == nullptr) {
        return Status::make(ErrorCode::eInvalidArgument, "writeFile: data is null", path, "BinaryIO");
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
 * @brief Read a text header terminated by the first blank line.
 * @param f Open input stream (positioned at start of header).
 * @param header_text Output: all header bytes up to and including the blank line.
 * @param data_offset Output: absolute offset in file where the binary payload starts.
 * @return Status (eOk when blank line found, eDataTruncated if EOF before blank line).
 */
[[nodiscard]] inline Status readHeaderUntilBlankLine(std::ifstream& f, std::string& header_text, std::streamoff& data_offset) {
    header_text.clear();
    data_offset = 0;
    if (!f) {
        return Status::make(ErrorCode::eFileReadFailed, "readHeaderUntilBlankLine: invalid stream", {}, "BinaryIO");
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

/**
 * @brief Byte-swap a single element in place (e.g. for big-endian data).
 * @param bytes Pointer to element bytes.
 * @param elem_size Element size in bytes (e.g. 2 for uint16).
 */
inline void byteSwapInPlace(uint8_t* bytes, int elem_size) {
    for (int j = 0; j < elem_size / 2; ++j) {
        const int k = elem_size - 1 - j;
        std::swap(bytes[j], bytes[k]);
    }
}

/**
 * @brief Byte-swap all elements in a buffer in place.
 * @param buf Buffer of contiguous elements.
 * @param elem_size Size of each element in bytes.
 */
inline void byteSwapBufferInPlace(std::vector<uint8_t>& buf, int elem_size) {
    if (elem_size <= 1) {
        return;
    }
    const size_t n = buf.size() / static_cast<size_t>(elem_size);
    for (size_t i = 0; i < n; ++i) {
        byteSwapInPlace(buf.data() + i * static_cast<size_t>(elem_size), elem_size);
    }
}

}  // namespace binaryio
}  // namespace io
}  // namespace vne
