#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ---------------------------------------------------------------------- */

#include "vertexnova/io/export.h"
#include "vertexnova/io/image/gzip_flat_volume_loader.h"
#include "vertexnova/io/image/tar_slice_volume_loader.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_format.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne {
namespace image {

/**
 * @brief Optional overrides merged with auto-detected layout metadata.
 *
 * Leave dimensions at 0 and strings empty to fully auto-detect. Non-zero / non-empty fields
 * override the detected values (useful for spacing or known dataset presets in applications).
 */
struct VolumeLoadHints {
    VolumeLoadRoute format = VolumeLoadRoute::eUnknown;
    FlatVolumeLayout flat{};
    TarSliceVolumeLayout tar_slice{};
};

/**
 * @brief Routes volume loading by path classification (@ref parseVolumePath).
 *
 * - Semantic: @c .nrrd / @c .mhd (self-describing headers)
 * - Wrappers: @c .tar / @c .tar.gz → slice-stack layout in tar
 * - Wrappers: @c .bin-gz → dense grid in gzip (not bare @c .gz)
 */
class VNEIO_API VolumeLoader {
   public:
    [[nodiscard]] bool load(const std::string& path, Volume& out_volume, const VolumeLoadHints& hints = {});
    [[nodiscard]] vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request);
    [[nodiscard]] const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
