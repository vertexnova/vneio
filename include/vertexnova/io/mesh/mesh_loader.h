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
#include "mesh.h"

namespace VNE {
namespace Mesh {

/**
 * @brief Interface for loading 3D meshes from file
 *
 * Implementations (e.g. AssimpLoader) load supported formats into Mesh.
 * Use MeshLoaderRegistry to obtain a loader by file extension.
 */
class IMeshLoader {
   public:
    virtual ~IMeshLoader() = default;

    /**
     * @brief Load a mesh from file
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

}  // namespace Mesh
}  // namespace VNE
