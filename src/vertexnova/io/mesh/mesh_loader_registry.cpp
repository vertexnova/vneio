/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/mesh/mesh_loader_registry.h"
#include "vertexnova/io/mesh/assimp_loader.h"

namespace vne {
namespace mesh {

std::unique_ptr<IMeshLoader> MeshLoaderRegistry::getLoaderFor(const std::string& path) {
    AssimpLoader checker;
    if (checker.isExtensionSupported(path)) {
        return std::make_unique<AssimpLoader>();
    }
    return nullptr;
}

}  // namespace mesh
}  // namespace vne
