#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Base interface for asset loaders. Used by AssetIO registry.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/load_request.h"

namespace VNE {
namespace IO {

/**
 * @brief Base interface for asset loaders
 */
class IAssetLoader {
   public:
    virtual ~IAssetLoader() = default;

    /**
     * @brief Check if this loader can handle the given request
     * @param request Load request (uri, hint_format, asset_type)
     * @return true if this loader can load the asset
     */
    [[nodiscard]] virtual bool canLoad(const LoadRequest& request) const = 0;
};

}  // namespace IO
}  // namespace VNE
