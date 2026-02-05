#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/common/status.h"

#include <string>

namespace vne {
namespace io {

/**
 * @brief Asset kind for load requests
 */
enum class AssetType : uint8_t {
    eImage = 0,
    eMesh,
    eVolume,
    eDicomSeries,
};

/**
 * @brief Request to load an asset (file path or future: URI, pak, etc.)
 */
struct LoadRequest {
    AssetType asset_type = AssetType::eImage;
    std::string uri;
    std::string hint_format;     // e.g. "png", "gltf", "nrrd", "mhd", "dicom"
    bool generate_mips = false;  // for images
    bool force_srgb = false;     // for images
    bool prefer_16bit = false;   // for medical volumes if applicable
};

/**
 * @brief Load result: asset + status. Alias to Result for consistency with loader APIs.
 */
template<typename T>
using LoadResult = Result<T>;

}  // namespace io
}  // namespace vne
