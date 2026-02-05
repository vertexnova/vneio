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

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/image/image.h"

namespace vne {
namespace image {

/**
 * @file image_loader.h
 * @brief Interface for loading 2D images (PNG, JPG, BMP, TGA, etc.).
 */

/**
 * @class IImageLoader
 * @brief Interface for loading 2D images.
 */
class IImageLoader : public vne::io::IAssetLoader {
   public:
    ~IImageLoader() override = default;

    /**
     * @brief Load an image from the given request
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Image on success, Status on failure
     */
    virtual vne::io::LoadResult<Image> loadImage(const vne::io::LoadRequest& request) = 0;
};

}  // namespace image
}  // namespace vne
