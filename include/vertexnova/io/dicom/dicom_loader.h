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

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/dicom/dicom_series.h"
#include "vertexnova/io/load_request.h"

namespace vne::dicom {

class IDicomLoader : public vne::io::IAssetLoader {
   public:
    ~IDicomLoader() override = default;

    bool canLoad(const vne::io::LoadRequest& request) const override;

    /**
     * @brief Load a DICOM series from the given request (AssetIO registry API)
     * @param request Load request (uri = directory path, hint_format optional)
     * @return Load result with DicomSeries_C on success, Status on failure
     */
    virtual vne::io::LoadResult<DicomSeries_C> loadDicomSeries(const vne::io::LoadRequest& request) = 0;

    /** @brief Load a series from a directory containing DICOM slices (legacy API). */
    virtual bool loadDirectory(const std::string& directory_path, DicomSeries_C& out_series) = 0;

    /** @brief Load a specific series UID from the directory (multi-series folders). */
    virtual bool loadDirectorySeries(const std::string& directory_path,
                                     const std::string& series_uid,
                                     DicomSeries_C& out_series) {
        (void)series_uid;
        return loadDirectory(directory_path, out_series);
    }

    virtual std::string getLastError() const = 0;
};

inline bool IDicomLoader::canLoad(const vne::io::LoadRequest& request) const {
    return request.asset_type == vne::io::AssetType::eDicomSeries && !request.uri.empty();
}

}  // namespace vne::dicom
