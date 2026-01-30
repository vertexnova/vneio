#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Loader for MetaImage (MHD/MHA) 3D volumes. Reads header and
 * separate or embedded raw data. Supports MET_UCHAR, MET_USHORT, MET_FLOAT.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"

namespace vne {
namespace Image {

/**
 * @brief Loader for MetaImage (MHD/MHA) 3D volumes
 *
 * Reads NDims, DimSize, ElementType, ElementSpacing, ElementDataFile
 * (or inline data in MHA). ElementType: MET_UCHAR, MET_USHORT, MET_FLOAT.
 */
class MhdLoader : public IVolumeLoader {
   public:
    MhdLoader() = default;

    bool canLoad(const vne::io::LoadRequest& request) const override;
    vne::io::LoadResult<Volume> loadVolume(const vne::io::LoadRequest& request) override;

    bool load(const std::string& path, Volume& out_volume);
    bool isExtensionSupported(const std::string& path) const;
    const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace Image
}  // namespace vne
