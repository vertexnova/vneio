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

#include "vertexnova/io/common/status.h"

#include <string>

namespace vne {
namespace io {

/**
 * @file load_request.h
 * @brief Load request and result types for AssetIO registry and loader interfaces.
 */

/**
 * @enum AssetType
 * @brief Asset kind for load requests.
 */
enum class AssetType : uint8_t {
    eImage = 0,      //!< 2D image (PNG, JPG, etc.)
    eMesh,           //!< 3D mesh (OBJ, STL, glTF, etc.)
    eVolume,         //!< 3D volume (NRRD, MHD, etc.)
    eDicomSeries,    //!< DICOM series (directory of slices)
};

/**
 * @struct LoadRequest
 * @brief Request to load an asset (file path or future: URI, pak, etc.)
 */
struct LoadRequest {
    AssetType asset_type = AssetType::eImage;  //!< Kind of asset to load.
    std::string uri;                           //!< File path or resource URI.
    std::string hint_format;                   //!< Optional format hint (e.g. "png", "nrrd", "dicom").
    bool generate_mips = false;                 //!< For images: generate mipmaps.
    bool force_srgb = false;                   //!< For images: treat as sRGB.
    bool prefer_16bit = false;                 //!< For medical volumes: prefer 16-bit if applicable.
};

/**
 * @brief Load result: asset value + status. Alias to Result for consistency with loader APIs.
 */
template<typename T>
using LoadResult = Result<T>;

}  // namespace io
}  // namespace vne
