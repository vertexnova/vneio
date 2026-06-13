#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/export.h"
#include "vertexnova/io/load_request.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace vne {
namespace io {
namespace compression {

/**
 * @brief Decompress a gzip stream (including .gz and .bin-gz payloads).
 * @return Decompressed bytes or error status when ZLIB is unavailable or data is invalid.
 */
[[nodiscard]] VNEIO_API LoadResult<std::vector<std::uint8_t>> decompressGzip(std::span<const std::uint8_t> compressed);

/** @brief Read a file and gzip-decompress its contents. */
[[nodiscard]] VNEIO_API LoadResult<std::vector<std::uint8_t>> decompressGzipFile(const std::string& path);

/** @brief True when vneio was built with ZLIB gzip support. */
[[nodiscard]] VNEIO_API bool gzipSupported() noexcept;

}  // namespace compression
}  // namespace io
}  // namespace vne
