/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/asset_io.h"
#include "vertexnova/io/common/status.h"

namespace VNE {
namespace IO {

void AssetIO::registerImageLoader(std::unique_ptr<VNE::Image::IImageLoader> loader) {
    if (loader) {
        image_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerMeshLoader(std::unique_ptr<VNE::Mesh::IMeshLoader> loader) {
    if (loader) {
        mesh_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerVolumeLoader(std::unique_ptr<VNE::Image::IVolumeLoader> loader) {
    if (loader) {
        volume_loaders_.push_back(std::move(loader));
    }
}

void AssetIO::registerDicomLoader(std::unique_ptr<VNE::DICOM::IDicomLoader> loader) {
    if (loader) {
        dicom_loaders_.push_back(std::move(loader));
    }
}

LoadResult<VNE::Image::Image> AssetIO::loadImage(const LoadRequest& request) {
    LoadResult<VNE::Image::Image> result;
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

LoadResult<VNE::Mesh::Mesh> AssetIO::loadMesh(const LoadRequest& request) {
    LoadResult<VNE::Mesh::Mesh> result;
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

LoadResult<VNE::Image::Volume> AssetIO::loadVolume(const LoadRequest& request) {
    LoadResult<VNE::Image::Volume> result;
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

LoadResult<VNE::DICOM::DicomSeries_C> AssetIO::loadDicomSeries(const LoadRequest& request) {
    LoadResult<VNE::DICOM::DicomSeries_C> result;
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

}  // namespace IO
}  // namespace VNE
