#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/export.h"
#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/mesh/mesh.h"

#include <string>

namespace vne {
namespace mesh {

/**
 * @file mesh_loader.h
 * @brief Interface for loading 3D meshes from file; register with AssetIO for unified dispatch.
 */

/**
 * @class IMeshLoader
 * @brief Interface for loading 3D meshes; register with AssetIO for dispatch.
 *
 * Implement loadMesh() and isExtensionSupported(). Register with
 * AssetIO::registerMeshLoader(); AssetIO calls canLoad() and dispatches to
 * the first matching loader.
 */
class VNEIO_API IMeshLoader : public vne::io::IAssetLoader {
   public:
    ~IMeshLoader() override = default;

    [[nodiscard]] bool canLoad(const vne::io::LoadRequest& request) const override;

    /**
     * @brief Load a mesh from the given request.
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Mesh on success, Status on failure
     */
    [[nodiscard]] virtual vne::io::LoadResult<Mesh> loadMesh(const vne::io::LoadRequest& request) = 0;

    /**
     * @brief Check if this loader supports the given file (by extension)
     * @param path File path or filename
     * @return true if the extension is supported
     */
    [[nodiscard]] virtual bool isExtensionSupported(const std::string& path) const = 0;

    /**
     * @brief Last error message after a failed load
     */
    [[nodiscard]] virtual const std::string& getLastError() const = 0;
};

inline bool IMeshLoader::canLoad(const vne::io::LoadRequest& request) const {
    return request.asset_type == vne::io::AssetType::eMesh && isExtensionSupported(request.uri);
}

}  // namespace mesh
}  // namespace vne
