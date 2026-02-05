#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne {
namespace image {

/**
 * @brief Loader for NRRD 3D volumes
 *
 * Supports: dimension 3, type uchar/uint8/ushort/uint16/float,
 * encoding raw, attached or detached data file. spacings and
 * (optionally) space origin are read when present.
 */
class NrrdLoader : public IVolumeLoader {
   public:
    NrrdLoader() = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;
    [[nodiscard]] vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) override;

    [[nodiscard]] bool load(const std::string& path, Volume& out_volume);
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const;
    [[nodiscard]] const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
