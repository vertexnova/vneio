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

#include "vertexnova/io/dicom/dicom_loader.h"
#include "vertexnova/io/dicom/dicom_series.h"
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/image_loader.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"

#include <memory>
#include <vector>

namespace vne {
namespace io {

/**
 * @file asset_io.h
 * @brief Unified asset IO: register loaders and load by request (CPU decode only).
 */

/**
 * @class AssetIO
 * @brief Unified asset io: register loaders and load by request.
 *
 * Decode on CPU only; upload to GPU lives in a separate module (e.g. engine).
 */
class AssetIO {
   public:
    AssetIO() = default;
    ~AssetIO() = default;

    /** @brief Register an image loader. Called first is tried first. */
    void registerImageLoader(std::unique_ptr<vne::image::IImageLoader> loader);
    /** @brief Register a mesh loader. */
    void registerMeshLoader(std::unique_ptr<vne::mesh::IMeshLoader> loader);
    /** @brief Register a volume loader. */
    void registerVolumeLoader(std::unique_ptr<vne::image::IVolumeLoader> loader);
    /** @brief Register a DICOM loader. */
    void registerDicomLoader(std::unique_ptr<vne::dicom::IDicomLoader> loader);

    /**
     * @brief Load an image from the given request.
     * @param request Load request (uri = file path).
     * @return Load result with Image on success, Status on failure.
     */
    LoadResult<vne::image::Image> loadImage(const LoadRequest& request);
    /**
     * @brief Load a mesh from the given request.
     * @param request Load request (uri = file path).
     * @return Load result with Mesh on success, Status on failure.
     */
    LoadResult<vne::mesh::Mesh> loadMesh(const LoadRequest& request);
    /**
     * @brief Load a volume from the given request.
     * @param request Load request (uri = file path).
     * @return Load result with Volume on success, Status on failure.
     */
    LoadResult<vne::image::Volume> loadVolume(const LoadRequest& request);
    /**
     * @brief Load a DICOM series from the given request.
     * @param request Load request (uri = directory path).
     * @return Load result with DicomSeries on success, Status on failure.
     */
    LoadResult<vne::dicom::DicomSeries> loadDicomSeries(const LoadRequest& request);

   private:
    std::vector<std::unique_ptr<vne::image::IImageLoader>> image_loaders_;
    std::vector<std::unique_ptr<vne::mesh::IMeshLoader>> mesh_loaders_;
    std::vector<std::unique_ptr<vne::image::IVolumeLoader>> volume_loaders_;
    std::vector<std::unique_ptr<vne::dicom::IDicomLoader>> dicom_loaders_;
};

}  // namespace io
}  // namespace vne
