#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Unified asset IO registry. Registers loaders per asset kind and
 * dispatches load requests. CPU-only; no GPU handles.
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
 * @brief Unified asset io: register loaders and load by request
 *
 * Decode on CPU only; upload to GPU lives in a separate module (e.g. engine).
 */
class AssetIO {
   public:
    AssetIO() = default;
    ~AssetIO() = default;

    void registerImageLoader(std::unique_ptr<vne::image::IImageLoader> loader);
    void registerMeshLoader(std::unique_ptr<vne::mesh::IMeshLoader> loader);
    void registerVolumeLoader(std::unique_ptr<vne::image::IVolumeLoader> loader);
    void registerDicomLoader(std::unique_ptr<vne::dicom::IDicomLoader> loader);

    LoadResult<vne::image::Image> loadImage(const LoadRequest& request);
    LoadResult<vne::mesh::Mesh> loadMesh(const LoadRequest& request);
    LoadResult<vne::image::Volume> loadVolume(const LoadRequest& request);
    LoadResult<vne::dicom::DicomSeries> loadDicomSeries(const LoadRequest& request);

   private:
    std::vector<std::unique_ptr<vne::image::IImageLoader>> image_loaders_;
    std::vector<std::unique_ptr<vne::mesh::IMeshLoader>> mesh_loaders_;
    std::vector<std::unique_ptr<vne::image::IVolumeLoader>> volume_loaders_;
    std::vector<std::unique_ptr<vne::dicom::IDicomLoader>> dicom_loaders_;
};

}  // namespace io
}  // namespace vne
