/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/asset_io.h"
#include "vertexnova/io/common/status.h"

namespace vne {
namespace io {

void AssetIO::registerImageLoader(std::unique_ptr<vne::image::IImageLoader> loader) {
    if (loader) {
        image_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerMeshLoader(std::unique_ptr<vne::mesh::IMeshLoader> loader) {
    if (loader) {
        mesh_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerVolumeLoader(std::unique_ptr<vne::image::IVolumeLoader> loader) {
    if (loader) {
        volume_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerDicomLoader(std::unique_ptr<vne::dicom::IDicomLoader> loader) {
    if (loader) {
        dicom_loaders_.push_back(std::move(loader));
    }
}

LoadResult<vne::image::Image> AssetIO::loadImage(const LoadRequest& request) {
    LoadResult<vne::image::Image> result;
    for (const auto& loader : image_loaders_) {
        if (loader->canLoad(request)) {
            result = loader->loadImage(request);
            if (result.ok()) {
                return result;
            }
        }
    }
    if (!result.status.ok()) {
        return result;
    }
    result.status = Status::make(ErrorCode::eUnsupportedFormat,
                                  "No image loader could load: " + request.uri,
                                   request.uri,
                                   "AssetIO");
    return result;
}

LoadResult<vne::mesh::Mesh> AssetIO::loadMesh(const LoadRequest& request) {
    LoadResult<vne::mesh::Mesh> result;
    for (const auto& loader : mesh_loaders_) {
        if (loader->canLoad(request)) {
            result = loader->loadMesh(request);
            if (result.ok()) {
                return result;
            }
        }
    }
    if (!result.status.ok()) {
        return result;
    }
    result.status = Status::make(ErrorCode::eUnsupportedFormat,
                                  "No mesh loader could load: " + request.uri,
                                   request.uri,
                                   "AssetIO");
    return result;
}

LoadResult<vne::image::Volume> AssetIO::loadVolume(const LoadRequest& request) {
    LoadResult<vne::image::Volume> result;
    for (const auto& loader : volume_loaders_) {
        if (loader->canLoad(request)) {
            result = loader->loadVolume(request);
            if (result.ok()) {
                return result;
            }
        }
    }
    if (!result.status.ok()) {
        return result;
    }
    result.status = Status::make(ErrorCode::eUnsupportedFormat,
                                  "No volume loader could load: " + request.uri,
                                   request.uri,
                                   "AssetIO");
    return result;
}

LoadResult<vne::dicom::DicomSeries_C> AssetIO::loadDicomSeries(const LoadRequest& request) {
    LoadResult<vne::dicom::DicomSeries_C> result;
    for (const auto& loader : dicom_loaders_) {
        if (loader->canLoad(request)) {
            result = loader->loadDicomSeries(request);
            if (result.ok()) {
                return result;
            }
        }
    }
    if (!result.status.ok()) {
        return result;
    }
    result.status = Status::make(ErrorCode::eUnsupportedFormat,
                                 "No DICOM loader could load: " + request.uri,
                                   request.uri,
                                   "AssetIO");
    return result;
}

}  // namespace io
}  // namespace vne
