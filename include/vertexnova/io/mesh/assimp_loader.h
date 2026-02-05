#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   June 2025
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"

#include <string>

namespace vne {
namespace mesh {

/** Default fill ratio when normalizing mesh to unit sphere (slightly inside 1.0 to avoid clipping). */
constexpr float kAssimpNormalizeFillDefault = 0.999f;

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
    float normalize_fill = kAssimpNormalizeFillDefault;
    bool generate_barycentrics = false;
};

/**
 * @brief Loader for 3D meshes using Assimp (implements IMeshLoader)
 */
class AssimpLoader : public IMeshLoader {
   public:
    AssimpLoader() = default;
    ~AssimpLoader() override = default;

    [[nodiscard]] vne::io::LoadResult<Mesh> loadMesh(const vne::io::LoadRequest& request) override;
    [[nodiscard]] bool loadFile(const std::string& path, Mesh& out_mesh) override;
    [[nodiscard]] bool loadFile(const std::string& path, Mesh& out_mesh, const AssimpLoaderOptions& opts);
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const override;
    [[nodiscard]] const std::string& getLastError() const override { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace mesh
}  // namespace vne
