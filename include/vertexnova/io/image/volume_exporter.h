#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Volume export utilities (NRRD, MHD/MHA).
 *
 * These exporters are designed for deterministic test assets and tools.
 * They are not intended to replace full-featured medical io libraries.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/image/volume.h"

namespace vne::Image {

struct NrrdExportOptions {
    bool detached_data = false;      // if true: writes .nhdr + separate .raw
    std::string detached_data_name;  // optional override name for the raw file
};

struct MhdExportOptions {
    bool inline_data = false;   // if true: writes .mha (ElementDataFile = LOCAL)
    std::string raw_data_name;  // used when inline_data==false
};

bool ExportNrrd(const std::string& nrrd_or_nhdr_path,
                const Volume& vol,
                const NrrdExportOptions& opts = {},
                std::string* out_error = nullptr);
bool ExportMhd(const std::string& mhd_or_mha_path,
               const Volume& vol,
               const MhdExportOptions& opts = {},
               std::string* out_error = nullptr);

}  // namespace vne::Image
