#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne {
namespace image {

/**
 * @file mhd_loader.h
 * @brief Loader for MetaImage (MHD/MHA) 3D volumes.
 */

/**
 * @class MhdLoader
 * @brief Loader for MetaImage (MHD/MHA) 3D volumes (implements IVolumeLoader).
 *
 * Reads NDims, DimSize, ElementType, ElementSpacing, ElementDataFile
 * (or inline data in MHA). ElementType: MET_UCHAR, MET_USHORT, MET_FLOAT.
 */
class MhdLoader : public IVolumeLoader {
   public:
    MhdLoader() = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;
    [[nodiscard]] vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) override;

    /**
     * @brief Load a volume from a MHD/MHA file (legacy API).
     * @param path Path to .mhd or .mha file.
     * @param out_volume Volume to fill.
     * @return true on success, false otherwise (see getLastError()).
     */
    [[nodiscard]] bool load(const std::string& path, Volume& out_volume);
    /**
     * @brief Check if the path has a supported MHD/MHA extension.
     * @param path File path or filename.
     * @return true if supported.
     */
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const;
    [[nodiscard]] const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace image
}  // namespace vne
