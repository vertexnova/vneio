#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Interface for image loaders. Used by AssetIO registry.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/image/image.h"

namespace VNE {
namespace Image {

/**
 * @brief Interface for loading 2D images (PNG, JPG, etc.)
 */
class IImageLoader : public VNE::IO::IAssetLoader {
   public:
    ~IImageLoader() override = default;

    /**
     * @brief Load an image from the given request
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Image on success, Status_C on failure
     */
    virtual VNE::IO::LoadResult<Image> loadImage(const VNE::IO::LoadRequest& request) = 0;
};

}  // namespace Image
}  // namespace VNE
