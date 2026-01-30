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

namespace VNE {
namespace Image {

/**
 * @brief Interface for loading volumes (NRRD, MHD, etc.)
 */
class IVolumeLoader : public VNE::IO::IAssetLoader {
   public:
    ~IVolumeLoader() override = default;

    /**
     * @brief Load a volume from the given request
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Volume on success, Status_C on failure
     */
    virtual VNE::IO::LoadResult<Volume> loadVolume(const VNE::IO::LoadRequest& request) = 0;
};

}  // namespace Image
}  // namespace VNE
