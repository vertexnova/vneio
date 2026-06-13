/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/common/compression.h"

#include "vertexnova/io/common/binary_io.h"

#include <vector>

#if defined(VNEIO_HAS_ZLIB)
#include <zlib.h>
#endif

namespace vne {
namespace io {
namespace compression {

bool gzipSupported() noexcept {
#if defined(VNEIO_HAS_ZLIB)
    return true;
#else
    return false;
#endif
}

LoadResult<std::vector<std::uint8_t>> decompressGzip(std::span<const std::uint8_t> compressed) {
    LoadResult<std::vector<std::uint8_t>> result;
#if !defined(VNEIO_HAS_ZLIB)
    result.status = Status::make(ErrorCode::eNotImplemented,
                                 "gzip decompression requires ZLIB (VNEIO_HAS_ZLIB)",
                                 {},
                                 "Compression");
    return result;
#else
    if (compressed.empty()) {
        result.status = Status::make(ErrorCode::eInvalidArgument, "empty gzip input", {}, "Compression");
        return result;
    }

    z_stream stream{};
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        result.status = Status::make(ErrorCode::eThirdPartyError, "inflateInit2 failed", {}, "Compression");
        return result;
    }

    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(compressed.data()));
    stream.avail_in = static_cast<uInt>(compressed.size());

    std::vector<std::uint8_t> out;
    out.reserve(compressed.size() * 4U);
    std::vector<std::uint8_t> chunk(65536U);
    int zret = Z_OK;
    while (zret != Z_STREAM_END) {
        stream.next_out = chunk.data();
        stream.avail_out = static_cast<uInt>(chunk.size());
        zret = inflate(&stream, Z_NO_FLUSH);
        if (zret != Z_OK && zret != Z_STREAM_END && zret != Z_BUF_ERROR) {
            inflateEnd(&stream);
            result.status = Status::make(ErrorCode::eDataCorrupt,
                                         "gzip inflate failed (code " + std::to_string(zret) + ")",
                                         {},
                                         "Compression");
            return result;
        }
        const std::size_t produced = chunk.size() - stream.avail_out;
        if (produced > 0) {
            out.insert(out.end(), chunk.data(), chunk.data() + produced);
        }
        if (zret == Z_BUF_ERROR && stream.avail_out == chunk.size()) {
            break;
        }
    }
    inflateEnd(&stream);

    if (zret != Z_STREAM_END) {
        result.status = Status::make(ErrorCode::eDataCorrupt, "gzip stream truncated or corrupt", {}, "Compression");
        return result;
    }

    result.value = std::move(out);
    result.status = Status::okStatus();
    return result;
#endif
}

LoadResult<std::vector<std::uint8_t>> decompressGzipFile(const std::string& path) {
    LoadResult<std::vector<std::uint8_t>> result;
    std::vector<std::uint8_t> file_bytes;
    const Status read_st = binaryio::readFile(path, file_bytes);
    if (!read_st.ok()) {
        result.status = read_st;
        result.status.subsystem = "Compression";
        return result;
    }
    result = decompressGzip(file_bytes);
    if (!result.ok()) {
        result.status.path = path;
    }
    return result;
}

}  // namespace compression
}  // namespace io
}  // namespace vne
