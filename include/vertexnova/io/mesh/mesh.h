#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   October 2025
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include <string>
#include <vector>
#include <cstdint>

namespace vne::mesh {

/**
 * @file mesh.h
 * @brief Mesh data structures for 3D geometry (vertices, indices, materials, submeshes).
 */

/**
 * @struct VertexAttributes
 * @brief Vertex structure with position, normal, tangent, bitangent, UV and optional barycentric coordinates.
 */
struct VertexAttributes {
    float position[3];     //!< 3D position coordinates
    float normal[3];       //!< Normal vector
    float tangent[3];      //!< Tangent vector for normal mapping
    float bitangent[3];    //!< Bitangent vector for normal mapping
    float texcoord0[2];    //!< Primary UV coordinates
    float barycentric[3];  //!< (Optional) barycentric coords for wireframe (default 0)
};

/**
 * @struct Material
 * @brief Material slot containing name, base color, and optional texture path.
 */
struct Material {
    std::string name;                    //!< Material name
    std::string base_color_tex;          //!< Base color texture file path (optional)
    float base_color[4] = {1, 1, 1, 1};  //!< Base color RGBA values
};

/**
 * @struct Submesh
 * @brief Submesh defining a range of indices and material index.
 */
struct Submesh {
    uint32_t first_index = 0;     //!< First index in the index buffer
    uint32_t index_count = 0;     //!< Number of indices for this submesh
    uint32_t material_index = 0;  //!< Index into the materials array
};

/**
 * @struct Mesh
 * @brief Mesh for loading and managing 3D meshes.
 *
 * Supports multi-material meshes with vertex attributes for modern rendering pipelines.
 */
struct Mesh {
    std::string name;                        //!< Mesh name/path
    std::vector<VertexAttributes> vertices;  //!< Vertex data
    std::vector<uint32_t> indices;           //!< Index data (32-bit)
    std::vector<Submesh> parts;              //!< Submesh definitions
    std::vector<Material> materials;         //!< Material definitions

    bool has_normals = false;       //!< Whether mesh has normal vectors
    bool has_tangent = false;       //!< Whether mesh has tangent/bitangent vectors
    bool has_uv0 = false;           //!< Whether mesh has UV coordinates
    float aabb_min[3] = {0, 0, 0};  //!< Axis-aligned bounding box minimum
    float aabb_max[3] = {0, 0, 0};  //!< Axis-aligned bounding box maximum

    [[nodiscard]] bool hasTexcoords() const { return has_uv0; }
    [[nodiscard]] size_t getVertexCount() const { return vertices.size(); }
    [[nodiscard]] size_t getIndexCount() const { return indices.size(); }
    [[nodiscard]] size_t getSubmeshCount() const { return parts.size(); }
    [[nodiscard]] size_t getMaterialCount() const { return materials.size(); }
    [[nodiscard]] bool isEmpty() const { return vertices.empty() || indices.empty(); }
};

/** @brief Canonical CPU mesh type alias (for AssetIO / upload documentation). */
using MeshAsset = Mesh;

}  // namespace vne::mesh
