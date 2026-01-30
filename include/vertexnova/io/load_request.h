#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Load request and result types for asset IO. Used by AssetIO registry
 * and loader interfaces. Reuses Status_C / Result_T from common/status.h.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/common/status.h"

namespace VNE {
namespace IO {

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
 * @brief Load result: asset + status. Alias to Result_T for consistency with loader APIs.
 */
template<typename T>
using LoadResult = Result_T<T>;

}  // namespace IO
}  // namespace VNE
