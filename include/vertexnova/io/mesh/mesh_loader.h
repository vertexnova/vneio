#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Interface for mesh loaders. Implementations (e.g. AssimpLoader) fill
 * Mesh from file paths. Enables pluggable loaders for multibackend engines.
 * ----------------------------------------------------------------------
 */

#include <string>

#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/mesh/mesh.h"

namespace VNE {
namespace Mesh {

/**
 * @brief Interface for loading 3D meshes from file
 *
 * Implementations (e.g. AssimpLoader) load supported formats into Mesh.
 * Use MeshLoaderRegistry to obtain a loader by file extension.
 */
class IMeshLoader : public VNE::IO::IAssetLoader {
   public:
    ~IMeshLoader() override = default;

    bool canLoad(const VNE::IO::LoadRequest& request) const override;

    /**
     * @brief Load a mesh from the given request (AssetIO registry API)
     * @param request Load request (uri = file path, hint_format optional)
     * @return Load result with Mesh on success, Status on failure
     */
    virtual VNE::IO::LoadResult<Mesh> loadMesh(const VNE::IO::LoadRequest& request) = 0;

    /**
     * @brief Load a mesh from file (legacy API, unchanged)
     * @param path Path to the mesh file
     * @param out_mesh Mesh to fill
     * @return true if loading succeeded, false otherwise (see getLastError())
     */
    virtual bool loadFile(const std::string& path, Mesh& out_mesh) = 0;

    /**
     * @brief Check if this loader supports the given file (by extension)
     * @param path File path or filename
     * @return true if the extension is supported
     */
    virtual bool isExtensionSupported(const std::string& path) const = 0;

    /**
     * @brief Last error message after a failed load
     */
    virtual const std::string& getLastError() const = 0;
};

inline bool IMeshLoader::canLoad(const VNE::IO::LoadRequest& request) const {
    return request.asset_type == VNE::IO::AssetType::eMesh && isExtensionSupported(request.uri);
}

}  // namespace Mesh
}  // namespace VNE
