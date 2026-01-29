#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * DICOM loader interface.
 *
 * Product philosophy:
 *  - vneio provides a stable interface and data model
 *  - implementation is provided by optional backends:
 *      * GDCM (common OSS choice)
 *      * DCMTK
 *
 * If neither is compiled in, the default loader returns an error.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/dicom/dicom_series.h"

namespace VNE::DICOM {

class IDicomLoader {
   public:
    virtual ~IDicomLoader() = default;

    // Load a series from a directory containing DICOM slices.
    virtual bool loadDirectory(const std::string& directory_path, DicomSeries_C& out_series) = 0;

    // Optional: load a specific series UID from the directory (multi-series folders)
    virtual bool loadDirectorySeries(const std::string& directory_path,
                                     const std::string& series_uid,
                                     DicomSeries_C& out_series) {
        (void)series_uid;
        return loadDirectory(directory_path, out_series);
    }

    virtual std::string getLastError() const = 0;
};

}  // namespace VNE::DICOM
