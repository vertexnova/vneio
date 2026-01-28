#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Loader for NRRD (.nrrd, .nhdr) 3D volumes. Supports raw encoding
 * and types uint8, uint16, float. Attached and detached headers.
 * ----------------------------------------------------------------------
 */

#include <string>
#include "volume.h"

namespace VNE {
namespace Image {

/**
 * @brief Loader for NRRD 3D volumes
 *
 * Supports: dimension 3, type uchar/uint8/ushort/uint16/float,
 * encoding raw, attached or detached data file. spacings and
 * (optionally) space origin are read when present.
 */
class NrrdLoader {
   public:
    NrrdLoader() = default;

    bool load(const std::string& path, Volume& out_volume);
    bool isExtensionSupported(const std::string& path) const;
    const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace Image
}  // namespace VNE
