#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Image loader using stb_image. Implements IImageLoader for AssetIO registry.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/image_loader.h"
#include "vertexnova/io/load_request.h"

namespace vne {
namespace Image {

/**
 * @brief Loader for 2D images (PNG, JPG, BMP, TGA, etc.) using stb_image
 */
class StbImageLoader : public IImageLoader {
   public:
    StbImageLoader() = default;

    bool canLoad(const vne::io::LoadRequest& request) const override;
    vne::io::LoadResult<Image> loadImage(const vne::io::LoadRequest& request) override;

    /**
     * @brief Check if the given path has a supported image extension
     */
    static bool isExtensionSupported(const std::string& path);

   private:
    std::string last_error_;
};

}  // namespace Image
}  // namespace vne
