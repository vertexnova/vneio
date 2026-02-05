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

#include "vertexnova/io/mesh/mesh_loader.h"

#include <memory>
#include <string>

namespace vne {
namespace mesh {

/**
 * @file mesh_loader_registry.h
 * @brief Factory for mesh loaders by file path (extension).
 */

/**
 * @class MeshLoaderRegistry
 * @brief Factory for mesh loaders by file path.
 *
 * getLoaderFor(path) returns a loader that supports the file extension,
 * or nullptr if none is available. The default build registers Assimp
 * for common formats (e.g. .obj, .stl, .fbx, .gltf). Caller owns the returned loader.
 */
class MeshLoaderRegistry {
   public:
    /**
     * @brief Return a loader that supports the given path, or nullptr
     * @param path File path or filename (extension is used to select loader)
     * @return Loader instance (caller owns) or nullptr
     */
    static std::unique_ptr<IMeshLoader> getLoaderFor(const std::string& path);
};

}  // namespace mesh
}  // namespace vne
