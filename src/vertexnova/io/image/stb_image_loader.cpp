/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"
#include <algorithm>
#include <cctype>
#include <string>

namespace VNE {
namespace Image {

namespace {

std::string lowerExtension(const std::string& path) {
    auto pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    std::string ext = path.substr(pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext;
}

}  // namespace

bool StbImageLoader::isExtensionSupported(const std::string& path) {
    std::string ext = lowerExtension(path);
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".gif"
           || ext == ".psd" || ext == ".hdr";
}

bool StbImageLoader::canLoad(const VNE::IO::LoadRequest& request) const {
    if (request.asset_type != VNE::IO::AssetType::eImage) {
        return false;
    }
    return isExtensionSupported(request.uri);
}

VNE::IO::LoadResult<Image> StbImageLoader::loadImage(const VNE::IO::LoadRequest& request) {
    VNE::IO::LoadResult<Image> result;
    last_error_.clear();
    if (!result.value.loadFromFile(request.uri)) {
        last_error_ = "StbImageLoader: failed to load image: " + request.uri;
        result.status = VNE::IO::Status::make(VNE::IO::ErrorCode::eFileReadFailed,
                                               last_error_,
                                               request.uri,
                                               "StbImageLoader");
        return result;
    }
    result.status = VNE::IO::Status::okStatus();
    return result;
}

}  // namespace Image
}  // namespace VNE
