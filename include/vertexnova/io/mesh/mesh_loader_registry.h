#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Registry/factory for mesh loaders. Returns an appropriate loader for
 * a given file path (by extension). Enables multibackend engines to
 * load meshes without coupling to a specific implementation.
 * ----------------------------------------------------------------------
 */

#include <memory>
#include <string>
#include "mesh_loader.h"

namespace vne {
namespace mesh {

/**
 * @brief Factory for mesh loaders by file path
 *
 * getLoaderFor(path) returns a loader that supports the file extension,
 * or nullptr if none is available. The default build registers Assimp
 * for common formats (e.g. .obj, .stl, .fbx, .gltf). Ownership of the
 * returned loader is transferred to the caller.
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
