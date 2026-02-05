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

#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"

#include <string>

namespace vne {
namespace mesh {

/**
 * @file assimp_loader.h
 * @brief Loader for 3D meshes using Assimp (implements IMeshLoader).
 */

/** Default fill ratio when normalizing mesh to unit sphere (slightly inside 1.0 to avoid clipping). */
constexpr float kAssimpNormalizeFillDefault = 0.999f;

/**
 * @struct AssimpLoaderOptions
 * @brief Options for Assimp mesh loading (UV flip, tangents, triangulation, etc.).
 */
struct AssimpLoaderOptions {
    bool flip_uvs = true;                  //!< Flip texture V coordinate.
    bool gen_tangents = true;              //!< Generate tangent/bitangent for normal mapping.
    bool triangulate = true;              //!< Convert to triangles.
    bool calc_normals_if_missing = false; //!< Compute normals if absent.
    bool pre_transform_vertices = true;   //!< Apply node transforms to vertices.
    bool ensure_ccw_winding = true;       //!< Ensure counter-clockwise winding.
    bool normalize_to_unit_sphere = false;//!< Scale mesh to fit unit sphere.
    float normalize_target_radius = 1.0f; //!< Target radius when normalizing.
    float normalize_fill = kAssimpNormalizeFillDefault; //!< Fill ratio when normalizing.
    bool generate_barycentrics = false;   //!< Generate barycentric coordinates (e.g. for wireframe).
};

/**
 * @class AssimpLoader
 * @brief Loader for 3D meshes using Assimp (implements IMeshLoader).
 */
class AssimpLoader : public IMeshLoader {
   public:
    AssimpLoader() = default;
    ~AssimpLoader() override = default;

    [[nodiscard]] vne::io::LoadResult<Mesh> loadMesh(const vne::io::LoadRequest& request) override;
    [[nodiscard]] bool loadFile(const std::string& path, Mesh& out_mesh) override;
    /**
     * @brief Load a mesh from file with options.
     * @param path Path to the mesh file.
     * @param out_mesh Mesh to fill.
     * @param opts Loader options (UV flip, tangents, triangulation, etc.).
     * @return true on success, false otherwise (see getLastError()).
     */
    [[nodiscard]] bool loadFile(const std::string& path, Mesh& out_mesh, const AssimpLoaderOptions& opts);
    [[nodiscard]] bool isExtensionSupported(const std::string& path) const override;
    [[nodiscard]] const std::string& getLastError() const override { return last_error_; }

   private:
    std::string last_error_;
};

}  // namespace mesh
}  // namespace vne
