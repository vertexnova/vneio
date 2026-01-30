#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Loader for NRRD (.nrrd, .nhdr) 3D volumes. Uses Teem nrrdio library
 * when available (supports compressed/advanced formats), otherwise falls
 * back to built-in parser (raw encoding only).
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"

#ifdef VNEIO_USE_NRRDIO
// nrrdio headers (path may vary: nrrdio.h, NrrdIO.h, or nrrdio/nrrdio.h)
#if __has_include(<nrrdio.h>)
#include <nrrdio.h>
#elif __has_include(<NrrdIO.h>)
#include <NrrdIO.h>
#elif __has_include(<nrrdio/nrrdio.h>)
#include <nrrdio/nrrdio.h>
#else
#error "nrrdio header not found. Check include path."
#endif
#endif

namespace VNE {
namespace Image {

/**
 * @brief Loader for NRRD 3D volumes
 *
 * Supports: dimension 3, type uchar/uint8/ushort/uint16/float,
 * encoding raw, attached or detached data file. spacings and
 * (optionally) space origin are read when present.
 */
class NrrdLoader : public IVolumeLoader {
   public:
    NrrdLoader() = default;

    bool canLoad(const VNE::IO::LoadRequest& request) const override;
    VNE::IO::LoadResult<Volume> loadVolume(const VNE::IO::LoadRequest& request) override;

    bool load(const std::string& path, Volume& out_volume);
    bool isExtensionSupported(const std::string& path) const;
    const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace Image
}  // namespace VNE
