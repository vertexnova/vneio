#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/export.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne {
namespace image {

/** @brief Layout metadata for a dense voxel buffer stored as a single gzip stream. */
struct FlatVolumeLayout {
    int width = 0;
    int height = 0;
    int depth = 0;
    VolumePixelType pixel_type = VolumePixelType::eUint8;
    float spacing[3] = {1.0f, 1.0f, 1.0f};
};

/**
 * @brief Loads a dense voxel grid from a gzip-wrapped binary blob (`.bin-gz` convention).
 *
 * Gzip is a compression wrapper only — not a semantic volume format. Layout (WxHxD, pixel type)
 * comes from @ref FlatVolumeLayout and/or filename heuristics.
 */
class VNEIO_API GzipFlatVolumeLoader : public IVolumeLoader {
   public:
    GzipFlatVolumeLoader() = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;
    [[nodiscard]] vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) override;

    [[nodiscard]] bool load(const std::string& path, const FlatVolumeLayout& layout, Volume& out_volume);
    /** @brief Auto-detect layout from filename / gzip size, then load. */
    [[nodiscard]] bool load(const std::string& path, Volume& out_volume);
    /** @brief Infer @p out_layout from filename tokens and gzip uncompressed size. */
    [[nodiscard]] bool detectLayout(const std::string& path, FlatVolumeLayout& out_layout) const;
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const;
    [[nodiscard]] const std::string& getLastError() const { return last_error_; }

    /** @brief Parses @c hint_format as @c "WxHxD" or @c "uint8:WxHxD" / @c "uint16:WxHxD". */
    [[nodiscard]] static bool parseLayoutHint(const std::string& hint_format, FlatVolumeLayout& out_layout);

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
