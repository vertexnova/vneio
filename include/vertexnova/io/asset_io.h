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

namespace VNE {
namespace IO {

/**
 * @brief Unified asset IO: register loaders and load by request
 *
 * Decode on CPU only; upload to GPU lives in a separate module (e.g. engine).
 */
class AssetIO {
   public:
    AssetIO() = default;
    ~AssetIO() = default;

    void registerImageLoader(std::unique_ptr<VNE::Image::IImageLoader> loader);
    void registerMeshLoader(std::unique_ptr<VNE::Mesh::IMeshLoader> loader);
    void registerVolumeLoader(std::unique_ptr<VNE::Image::IVolumeLoader> loader);
    void registerDicomLoader(std::unique_ptr<VNE::DICOM::IDicomLoader> loader);

    LoadResult<VNE::Image::Image> loadImage(const LoadRequest& request);
    LoadResult<VNE::Mesh::Mesh> loadMesh(const LoadRequest& request);
    LoadResult<VNE::Image::Volume> loadVolume(const LoadRequest& request);
    LoadResult<VNE::DICOM::DicomSeries_C> loadDicomSeries(const LoadRequest& request);

   private:
    std::vector<std::unique_ptr<VNE::Image::IImageLoader>> image_loaders_;
    std::vector<std::unique_ptr<VNE::Mesh::IMeshLoader>> mesh_loaders_;
    std::vector<std::unique_ptr<VNE::Image::IVolumeLoader>> volume_loaders_;
    std::vector<std::unique_ptr<VNE::DICOM::IDicomLoader>> dicom_loaders_;
};

}  // namespace IO
}  // namespace VNE
