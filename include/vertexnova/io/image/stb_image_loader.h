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

#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/image_loader.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne {
namespace image {

/**
 * @file stb_image_loader.h
 * @brief Loader for 2D images (PNG, JPG, BMP, TGA, etc.) using stb_image.
 */

/**
 * @class StbImageLoader
 * @brief Loader for 2D images using stb_image (implements IImageLoader).
 */
class StbImageLoader : public IImageLoader {
   public:
    StbImageLoader() = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;
    [[nodiscard]] vne::io::LoadResult<Image> loadImage(const vne::io::LoadRequest& request) override;

    /**
     * @brief Check if the given path has a supported image extension.
     * @param path File path or filename.
     * @return true if the extension is supported.
     */
    [[nodiscard]] static bool isExtensionSupported(const std::string& path);

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
