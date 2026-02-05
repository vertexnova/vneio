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

#include "vertexnova/io/image/volume.h"

#include <string>

namespace vne::image {

/**
 * @file volume_exporter.h
 * @brief Export 3D volumes to NRRD and MHD/MHA (for tests and tools).
 */

/**
 * @struct NrrdExportOptions
 * @brief Options for NRRD export (detached vs attached data file).
 */
struct NrrdExportOptions {
    bool detached_data = false;       //!< If true, write .nhdr + separate .raw file.
    std::string detached_data_name;  //!< Override name for the raw file (optional).
};

/**
 * @struct MhdExportOptions
 * @brief Options for MHD/MHA export (inline vs separate raw file).
 */
struct MhdExportOptions {
    bool inline_data = false;   //!< If true, write .mha (ElementDataFile = LOCAL).
    std::string raw_data_name;  //!< Name for raw file when inline_data is false.
};

/**
 * @brief Export volume to NRRD (.nrrd or .nhdr + .raw).
 * @param nrrd_or_nhdr_path Output path for .nrrd or .nhdr.
 * @param vol Volume to export.
 * @param opts Export options.
 * @param out_error If non-null, receives error message on failure.
 * @return true on success, false otherwise.
 */
[[nodiscard]] bool exportNrrd(const std::string& nrrd_or_nhdr_path,
                              const Volume& vol,
                              const NrrdExportOptions& opts = {},
                              std::string* out_error = nullptr);

/**
 * @brief Export volume to MetaImage (.mhd/.mha or .mha with inline data).
 * @param mhd_or_mha_path Output path for .mhd or .mha.
 * @param vol Volume to export.
 * @param opts Export options.
 * @param out_error If non-null, receives error message on failure.
 * @return true on success, false otherwise.
 */
[[nodiscard]] bool exportMhd(const std::string& mhd_or_mha_path,
                             const Volume& vol,
                             const MhdExportOptions& opts = {},
                             std::string* out_error = nullptr);

}  // namespace vne::image
