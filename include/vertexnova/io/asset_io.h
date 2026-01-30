#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Unified asset IO registry. Registers loaders per asset kind and
 * dispatches load requests. CPU-only; no GPU handles.
 * ----------------------------------------------------------------------
 */

#include <memory>
#include <vector>

#include "vertexnova/io/dicom/dicom_loader.h"
#include "vertexnova/io/dicom/dicom_series.h"
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/image_loader.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"

namespace vne {
namespace io {

/**
 * @brief Unified asset io: register loaders and load by request
 *
 * Decode on CPU only; upload to GPU lives in a separate module (e.g. engine).
 */
class AssetIO {
   public:
    AssetIO() = default;
    ~AssetIO() = default;

    void registerImageLoader(std::unique_ptr<vne::Image::IImageLoader> loader);
    void registerMeshLoader(std::unique_ptr<vne::Mesh::IMeshLoader> loader);
    void registerVolumeLoader(std::unique_ptr<vne::Image::IVolumeLoader> loader);
    void registerDicomLoader(std::unique_ptr<vne::DICOM::IDicomLoader> loader);

    LoadResult<vne::Image::Image> loadImage(const LoadRequest& request);
    LoadResult<vne::Mesh::Mesh> loadMesh(const LoadRequest& request);
    LoadResult<vne::Image::Volume> loadVolume(const LoadRequest& request);
    LoadResult<vne::DICOM::DicomSeries_C> loadDicomSeries(const LoadRequest& request);

   private:
    std::vector<std::unique_ptr<vne::Image::IImageLoader>> image_loaders_;
    std::vector<std::unique_ptr<vne::Mesh::IMeshLoader>> mesh_loaders_;
    std::vector<std::unique_ptr<vne::Image::IVolumeLoader>> volume_loaders_;
    std::vector<std::unique_ptr<vne::DICOM::IDicomLoader>> dicom_loaders_;
};

}  // namespace io
}  // namespace vne
