#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Interface for volume loaders. Used by AssetIO registry.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/image/volume.h"

namespace vne {
namespace image {

/**
 * @brief Interface for loading volumes (NRRD, MHD, etc.)
 */
class IVolumeLoader : public vne::io::IAssetLoader {
   public:
    ~IVolumeLoader() override = default;

    /**
     * @brief Load a volume from the given request
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Volume on success, Status on failure
     */
    [[nodiscard]] virtual vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) = 0;
};

}  // namespace image
}  // namespace vne
