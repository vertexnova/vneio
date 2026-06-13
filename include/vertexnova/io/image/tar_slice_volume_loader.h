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

/**
 * @brief Layout for a tar/tar.gz archive of numbered 2D slice files stacked into a 3D volume.
 *
 * Slice members are resolved as @c member_subdirectory + member_prefix + index, where @p index
 * runs from @c first_slice_index for @c depth slices. Unspecified fields (0 or empty) can be
 * inferred via @ref TarSliceVolumeLoader::detectLayout.
 */
struct TarSliceVolumeLayout {
    int width = 0;
    int height = 0;
    int depth = 0;
    std::string member_prefix;
    std::string member_subdirectory;
    int first_slice_index = 1;
    VolumePixelType pixel_type = VolumePixelType::eUint16;
    bool big_endian = true;
    float spacing[3] = {1.0f, 1.0f, 1.0f};
};

/**
 * @brief Loads slice-stack volumes from USTAR tar archives (optionally gzip-wrapped).
 *
 * Tar/gzip are transport wrappers — not semantic volume formats. Voxel layout is a numbered
 * 2D slice stack inside the archive (`.tar`, `.tar.gz`, `.tgz`).
 */
class VNEIO_API TarSliceVolumeLoader : public IVolumeLoader {
   public:
    TarSliceVolumeLoader() = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;
    [[nodiscard]] vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) override;

    [[nodiscard]] bool load(const std::string& path, Volume& out_volume);
    [[nodiscard]] bool load(const std::string& path, const TarSliceVolumeLayout& layout, Volume& out_volume);
    [[nodiscard]] bool detectLayout(const std::string& path, TarSliceVolumeLayout& inout_layout) const;
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const;
    [[nodiscard]] const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
