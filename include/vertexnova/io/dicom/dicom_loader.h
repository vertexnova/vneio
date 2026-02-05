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

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/dicom/dicom_series.h"
#include "vertexnova/io/load_request.h"

#include <string>

namespace vne::dicom {

/**
 * @file dicom_loader.h
 * @brief DICOM loader interface; implementations (GDCM/DCMTK) selected at link time.
 */

/**
 * @class IDicomLoader
 * @brief Interface for loading DICOM series from a directory.
 */
class IDicomLoader : public vne::io::IAssetLoader {
   public:
    ~IDicomLoader() override = default;

    bool canLoad(const vne::io::LoadRequest& request) const override;

    /**
     * @brief Load a DICOM series from the given request (AssetIO registry API).
     * @param request Load request (uri = directory path).
     * @return Load result with DicomSeries on success, Status on failure.
     */
    [[nodiscard]] virtual vne::io::LoadResult<DicomSeries> loadDicomSeries(const vne::io::LoadRequest& request) = 0;

    /**
     * @brief Load a series from a directory containing DICOM slices (legacy API).
     * @param directory_path Path to directory of DICOM files.
     * @param out_series Output series (volume + meta).
     * @return true on success, false otherwise (see getLastError()).
     */
    [[nodiscard]] virtual bool loadDirectory(const std::string& directory_path, DicomSeries& out_series) = 0;

    /**
     * @brief Load a specific series UID from the directory (multi-series folders).
     * @param directory_path Path to directory of DICOM files.
     * @param series_uid Series Instance UID to load (default impl ignores and uses loadDirectory).
     * @param out_series Output series.
     * @return true on success.
     */
    [[nodiscard]] virtual bool loadDirectorySeries(const std::string& directory_path,
                                                   const std::string& series_uid,
                                                   DicomSeries& out_series) {
        (void)series_uid;
        return loadDirectory(directory_path, out_series);
    }

    [[nodiscard]] virtual std::string getLastError() const = 0;
};

[[nodiscard]] inline bool IDicomLoader::canLoad(const vne::io::LoadRequest& request) const {
    return request.asset_type == vne::io::AssetType::eDicomSeries && !request.uri.empty();
}

}  // namespace vne::dicom
