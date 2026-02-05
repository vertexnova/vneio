#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/load_request.h"

namespace vne {
namespace io {

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

}  // namespace io
}  // namespace vne
