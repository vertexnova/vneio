#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   June 2025
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include <string>
#include "mesh.h"

namespace VNE {
namespace Mesh {

/**
 * @brief Options for Assimp mesh loading
 */
struct AssimpLoaderOptions {
    bool flip_uvs = true;
    bool gen_tangents = true;
    bool triangulate = true;
    bool calc_normals_if_missing = false;
    bool pre_transform_vertices = true;
    bool ensure_ccw_winding = true;
    bool normalize_to_unit_sphere = false;
    float normalize_target_radius = 1.0f;
    float normalize_fill = 0.999f;
    bool generate_barycentrics = false;
};

/**
 * @brief Loader for 3D meshes using Assimp
 */
class AssimpLoader {
   public:
    AssimpLoader() = default;
    ~AssimpLoader() = default;

    bool loadFile(const std::string& path, Mesh& out_mesh, const AssimpLoaderOptions& opts = {});
    static bool isExtensionSupported(const std::string& path);
    const std::string& getLastError() const { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace Mesh
}  // namespace VNE
