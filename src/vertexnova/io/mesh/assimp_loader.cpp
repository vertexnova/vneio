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

#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"

#ifdef VNEIO_NO_LOGGING
#include <iostream>
namespace VNE {
struct NullStream {
    template<typename T>
    NullStream& operator<<(const T&) {
        return *this;
    }
    NullStream& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};
inline NullStream null_stream;
}  // namespace VNE
#define CREATE_VNE_LOGGER_CATEGORY(name)
#define VNE_LOG_INFO VNE::null_stream
#define VNE_LOG_DEBUG VNE::null_stream
#define VNE_LOG_WARN VNE::null_stream
#define VNE_LOG_ERROR VNE::null_stream
#else
#include <vertexnova/logging/logging.h>
#endif

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/version.h>
#include <assimp/material.h>
#include <limits>
#include <algorithm>
#include <cmath>

namespace {
CREATE_VNE_LOGGER_CATEGORY("vne.core.mesh.assimp");

/**
 * @brief Build Assimp processing flags from options
 * @param opts Loading options
 * @return Assimp processing flags
 */
uint32_t BuildAssimpFlags(const VNE::Mesh::AssimpLoaderOptions& opts) {
    unsigned int flags = 0;

    if (opts.triangulate) {
        flags |= aiProcess_Triangulate;
    }
    if (opts.flip_uvs) {
        flags |= aiProcess_FlipUVs;
    }
    if (opts.gen_tangents) {
        flags |= aiProcess_CalcTangentSpace;
    }
    if (opts.calc_normals_if_missing) {
        flags |= aiProcess_GenSmoothNormals;
    }
    if (opts.pre_transform_vertices) {
        flags |= aiProcess_PreTransformVertices;
    }
    // Only use FlipWindingOrder if we're not pre-transforming vertices
    // PreTransformVertices already handles coordinate system conversion
    if (opts.ensure_ccw_winding && !opts.pre_transform_vertices) {
        flags |= aiProcess_FlipWindingOrder;
    }

    // Additional optimization flags
    flags |= aiProcess_JoinIdenticalVertices;
    flags |= aiProcess_ImproveCacheLocality;
    flags |= aiProcess_SortByPType;
    flags |= aiProcess_RemoveRedundantMaterials;
    flags |= aiProcess_OptimizeMeshes;
    // Only use OptimizeGraph if we're not pre-transforming vertices
    if (!opts.pre_transform_vertices) {
        flags |= aiProcess_OptimizeGraph;
    }

    return flags;
}
}  // namespace

namespace VNE {
namespace Mesh {

VNE::IO::LoadResult<Mesh> AssimpLoader::loadMesh(const VNE::IO::LoadRequest& request) {
    VNE::IO::LoadResult<Mesh> result;
    if (!loadFile(request.uri, result.value)) {
        result.status =
            VNE::IO::Status::make(VNE::IO::ErrorCode::eParseError, getLastError(), request.uri, "AssimpLoader");
        return result;
    }
    result.status = VNE::IO::Status::okStatus();
    return result;
}

bool AssimpLoader::loadFile(const std::string& path, Mesh& out_mesh) {
    return loadFile(path, out_mesh, AssimpLoaderOptions{});
}

bool AssimpLoader::loadFile(const std::string& path, Mesh& out_mesh, const AssimpLoaderOptions& opts) {
    last_error_.clear();

    Assimp::Importer importer;
    const unsigned int flags = BuildAssimpFlags(opts);

    VNE_LOG_INFO << "Loading mesh from: " << path;
    VNE_LOG_INFO << "Using Assimp version: " << aiGetVersionMajor() << "." << aiGetVersionMinor() << "."
                 << aiGetVersionRevision();
    VNE_LOG_DEBUG << "Assimp processing flags: 0x" << std::hex << flags << std::dec;

    const aiScene* scene = importer.ReadFile(path, flags);
    if (!scene || !scene->mRootNode) {
        last_error_ = "Assimp failed to load file: " + std::string(importer.GetErrorString());
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    VNE_LOG_INFO << "Successfully loaded scene with " << scene->mNumMeshes << " meshes and " << scene->mNumMaterials
                 << " materials";

    // Clear output mesh
    out_mesh.vertices.clear();
    out_mesh.indices.clear();
    out_mesh.parts.clear();
    out_mesh.materials.clear();
    out_mesh.name = path;

    // Initialize capability flags
    out_mesh.has_normals = false;
    out_mesh.has_tangent = false;
    out_mesh.has_uv0 = false;

    // Initialize AABB bounds
    out_mesh.aabb_min[0] = out_mesh.aabb_min[1] = out_mesh.aabb_min[2] = std::numeric_limits<float>::infinity();
    out_mesh.aabb_max[0] = out_mesh.aabb_max[1] = out_mesh.aabb_max[2] = -std::numeric_limits<float>::infinity();

    // Load materials
    out_mesh.materials.reserve(scene->mNumMaterials);
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* aim = scene->mMaterials[i];
        Material mat{};

        // Get material name
        aiString name;
        if (AI_SUCCESS == aim->Get(AI_MATKEY_NAME, name)) {
            mat.name = name.C_Str();
        }

        // Get base color (try PBR then classic)
        aiColor4D base{1, 1, 1, 1};
        if (AI_SUCCESS != aim->Get(AI_MATKEY_BASE_COLOR, base)) {
            aim->Get(AI_MATKEY_COLOR_DIFFUSE, base);
        }
        mat.base_color[0] = base.r;
        mat.base_color[1] = base.g;
        mat.base_color[2] = base.b;
        mat.base_color[3] = base.a;

        // Get base color texture (try PBR then classic)
        aiString tex_path;
        if (AI_SUCCESS != aim->GetTexture(aiTextureType_BASE_COLOR, 0, &tex_path)) {
            aim->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path);
        }
        mat.base_color_tex = tex_path.C_Str();

        out_mesh.materials.push_back(std::move(mat));
    }

    // Process meshes - concatenate into one vertex/index array, track submesh ranges
    uint32_t base_vertex = 0;

    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* am = scene->mMeshes[m];

        // Skip non-triangle meshes or empty meshes
        if (!(am->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) || am->mNumVertices == 0) {
            VNE_LOG_WARN << "Skipping mesh " << m << " - not triangulated or empty";
            continue;
        }

        VNE_LOG_DEBUG << "Processing mesh " << m << " with " << am->mNumVertices << " vertices and " << am->mNumFaces
                      << " faces";

        // Check vertex attributes
        const bool has_uv0 = am->HasTextureCoords(0);
        const bool has_normals = am->HasNormals();
        const bool has_tangent = am->HasTangentsAndBitangents();

        // OR across submeshes so top-level flags reflect the whole mesh
        out_mesh.has_uv0 = out_mesh.has_uv0 || has_uv0;
        out_mesh.has_normals = out_mesh.has_normals || has_normals;
        out_mesh.has_tangent = out_mesh.has_tangent || (has_tangent && has_uv0);

        VNE_LOG_DEBUG << "Mesh attributes - UVs: " << has_uv0 << ", Normals: " << has_normals
                      << ", Tangents: " << has_tangent;

        // Copy vertices
        const uint32_t vcount = am->mNumVertices;
        out_mesh.vertices.reserve(out_mesh.vertices.size() + vcount);

        const uint32_t start_index = static_cast<uint32_t>(out_mesh.indices.size());
        out_mesh.indices.reserve(out_mesh.indices.size() + 3u * am->mNumFaces);

        for (uint32_t v = 0; v < vcount; ++v) {
            VertexAttributes vv{};

            // Position
            const aiVector3D& p = am->mVertices[v];
            vv.position[0] = p.x;
            vv.position[1] = p.y;
            vv.position[2] = p.z;

            // Update AABB bounds
            out_mesh.aabb_min[0] = std::min(out_mesh.aabb_min[0], vv.position[0]);
            out_mesh.aabb_min[1] = std::min(out_mesh.aabb_min[1], vv.position[1]);
            out_mesh.aabb_min[2] = std::min(out_mesh.aabb_min[2], vv.position[2]);
            out_mesh.aabb_max[0] = std::max(out_mesh.aabb_max[0], vv.position[0]);
            out_mesh.aabb_max[1] = std::max(out_mesh.aabb_max[1], vv.position[1]);
            out_mesh.aabb_max[2] = std::max(out_mesh.aabb_max[2], vv.position[2]);

            // Normal
            if (has_normals) {
                const aiVector3D& n = am->mNormals[v];
                vv.normal[0] = n.x;
                vv.normal[1] = n.y;
                vv.normal[2] = n.z;
            } else {
                vv.normal[0] = 0.0f;
                vv.normal[1] = 1.0f;
                vv.normal[2] = 0.0f;
            }

            // Tangent and bitangent (only if UVs exist)
            if (has_tangent && has_uv0) {
                const aiVector3D& t = am->mTangents[v];
                const aiVector3D& b = am->mBitangents[v];
                vv.tangent[0] = t.x;
                vv.tangent[1] = t.y;
                vv.tangent[2] = t.z;
                vv.bitangent[0] = b.x;
                vv.bitangent[1] = b.y;
                vv.bitangent[2] = b.z;
            } else {
                vv.tangent[0] = 1.0f;
                vv.tangent[1] = 0.0f;
                vv.tangent[2] = 0.0f;
                vv.bitangent[0] = 0.0f;
                vv.bitangent[1] = 0.0f;
                vv.bitangent[2] = 1.0f;
            }

            // UV coordinates
            if (has_uv0) {
                const aiVector3D& uv = am->mTextureCoords[0][v];
                vv.texcoord0[0] = uv.x;
                vv.texcoord0[1] = uv.y;
            } else {
                vv.texcoord0[0] = 0.0f;
                vv.texcoord0[1] = 0.0f;
            }
            vv.barycentric[0] = 0.0f;
            vv.barycentric[1] = 0.0f;
            vv.barycentric[2] = 0.0f;
            out_mesh.vertices.push_back(vv);
        }

        // Copy indices
        for (uint32_t f = 0; f < am->mNumFaces; ++f) {
            const aiFace& face = am->mFaces[f];
            if (face.mNumIndices != 3) {
                VNE_LOG_WARN << "Skipping non-triangular face with " << face.mNumIndices << " indices";
                continue;
            }

            const uint32_t i0 = base_vertex + face.mIndices[0];
            const uint32_t i1 = base_vertex + face.mIndices[1];
            const uint32_t i2 = base_vertex + face.mIndices[2];
            out_mesh.indices.push_back(i0);
            out_mesh.indices.push_back(i1);
            out_mesh.indices.push_back(i2);
        }

        // Create submesh
        Submesh part{};
        part.first_index = start_index;
        part.index_count = static_cast<uint32_t>(out_mesh.indices.size()) - start_index;
        part.material_index = am->mMaterialIndex;
        out_mesh.parts.push_back(part);

        base_vertex += vcount;
    }

    const bool success = !out_mesh.vertices.empty() && !out_mesh.indices.empty();

    if (success) {
        VNE_LOG_INFO << "Successfully loaded mesh with " << out_mesh.getVertexCount() << " vertices, "
                     << out_mesh.getIndexCount() << " indices, " << out_mesh.getSubmeshCount() << " submeshes, and "
                     << out_mesh.getMaterialCount() << " materials";
        VNE_LOG_INFO << "AABB min: [" << out_mesh.aabb_min[0] << ", " << out_mesh.aabb_min[1] << ", "
                     << out_mesh.aabb_min[2] << "], max: [" << out_mesh.aabb_max[0] << ", " << out_mesh.aabb_max[1]
                     << ", " << out_mesh.aabb_max[2] << "]";
    } else {
        last_error_ = "Failed to load any valid mesh data";
        VNE_LOG_ERROR << last_error_;
        return false;
    }

    // ---- Optional: normalize mesh to a canonical size ----
    if (opts.normalize_to_unit_sphere && !out_mesh.vertices.empty()) {
        const float min_x = out_mesh.aabb_min[0];
        const float min_y = out_mesh.aabb_min[1];
        const float min_z = out_mesh.aabb_min[2];
        const float max_x = out_mesh.aabb_max[0];
        const float max_y = out_mesh.aabb_max[1];
        const float max_z = out_mesh.aabb_max[2];

        const float cx = 0.5f * (min_x + max_x);
        const float cy = 0.5f * (min_y + max_y);
        const float cz = 0.5f * (min_z + max_z);

        const float dx = (max_x - min_x);
        const float dy = (max_y - min_y);
        const float dz = (max_z - min_z);
        const float radius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);

        if (radius > 1e-6f && std::isfinite(radius)) {
            const float fill =
                (opts.normalize_fill > 0.0f && opts.normalize_fill <= 1.0f) ? opts.normalize_fill : 0.999f;
            const float target_r = std::max(1e-6f, opts.normalize_target_radius);
            const float scale = (target_r * fill) / radius;

            out_mesh.aabb_min[0] = out_mesh.aabb_min[1] = out_mesh.aabb_min[2] = std::numeric_limits<float>::infinity();
            out_mesh.aabb_max[0] = out_mesh.aabb_max[1] = out_mesh.aabb_max[2] =
                -std::numeric_limits<float>::infinity();

            for (auto& v : out_mesh.vertices) {
                v.position[0] = (v.position[0] - cx) * scale;
                v.position[1] = (v.position[1] - cy) * scale;
                v.position[2] = (v.position[2] - cz) * scale;

                out_mesh.aabb_min[0] = std::min(out_mesh.aabb_min[0], v.position[0]);
                out_mesh.aabb_min[1] = std::min(out_mesh.aabb_min[1], v.position[1]);
                out_mesh.aabb_min[2] = std::min(out_mesh.aabb_min[2], v.position[2]);
                out_mesh.aabb_max[0] = std::max(out_mesh.aabb_max[0], v.position[0]);
                out_mesh.aabb_max[1] = std::max(out_mesh.aabb_max[1], v.position[1]);
                out_mesh.aabb_max[2] = std::max(out_mesh.aabb_max[2], v.position[2]);
            }

            VNE_LOG_INFO << "Normalized mesh to unit sphere: center=[" << cx << "," << cy << "," << cz
                         << "], radius=" << radius << ", scale=" << scale;
        } else {
            VNE_LOG_WARN << "normalize_to_unit_sphere requested, but mesh bounds radius is degenerate: " << radius;
        }
    }

    // ---- Optional: generate barycentrics for wireframe rendering ----
    if (opts.generate_barycentrics) {
        if (out_mesh.indices.size() % 3 != 0) {
            VNE_LOG_WARN << "generate_barycentrics requested, but index count is not divisible by 3. Skipping.";
        } else {
            VNE_LOG_INFO << "Generating barycentrics (de-indexing triangles) for wireframe support";

            std::vector<VertexAttributes> new_vertices;
            std::vector<uint32_t> new_indices;
            std::vector<Submesh> new_parts;

            new_vertices.reserve(out_mesh.indices.size());
            new_indices.reserve(out_mesh.indices.size());
            new_parts.reserve(out_mesh.parts.size());

            auto emit_tri = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
                const uint32_t base = static_cast<uint32_t>(new_vertices.size());

                VertexAttributes v0 = out_mesh.vertices[i0];
                VertexAttributes v1 = out_mesh.vertices[i1];
                VertexAttributes v2 = out_mesh.vertices[i2];

                v0.barycentric[0] = 1.0f;
                v0.barycentric[1] = 0.0f;
                v0.barycentric[2] = 0.0f;
                v1.barycentric[0] = 0.0f;
                v1.barycentric[1] = 1.0f;
                v1.barycentric[2] = 0.0f;
                v2.barycentric[0] = 0.0f;
                v2.barycentric[1] = 0.0f;
                v2.barycentric[2] = 1.0f;

                new_vertices.push_back(v0);
                new_vertices.push_back(v1);
                new_vertices.push_back(v2);

                new_indices.push_back(base + 0);
                new_indices.push_back(base + 1);
                new_indices.push_back(base + 2);
            };

            if (out_mesh.parts.empty()) {
                Submesh part{};
                part.first_index = 0;
                part.material_index = 0;

                for (size_t i = 0; i < out_mesh.indices.size(); i += 3) {
                    emit_tri(out_mesh.indices[i + 0], out_mesh.indices[i + 1], out_mesh.indices[i + 2]);
                }

                part.index_count = static_cast<uint32_t>(new_indices.size());
                new_parts.push_back(part);
            } else {
                for (const auto& part : out_mesh.parts) {
                    Submesh np{};
                    np.first_index = static_cast<uint32_t>(new_indices.size());
                    np.material_index = part.material_index;

                    const uint32_t end = part.first_index + part.index_count;
                    for (uint32_t i = part.first_index; i + 2 < end; i += 3) {
                        emit_tri(out_mesh.indices[i + 0], out_mesh.indices[i + 1], out_mesh.indices[i + 2]);
                    }

                    np.index_count = static_cast<uint32_t>(new_indices.size()) - np.first_index;
                    new_parts.push_back(np);
                }
            }

            out_mesh.vertices = std::move(new_vertices);
            out_mesh.indices = std::move(new_indices);
            out_mesh.parts = std::move(new_parts);

            VNE_LOG_INFO << "Barycentric generation complete. New mesh: " << out_mesh.getVertexCount() << " vertices, "
                         << out_mesh.getIndexCount() << " indices";
        }
    }

    return success;
}

namespace {
bool AssimpIsExtensionSupported(const std::string& path) {
    const auto pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return false;
    }
    Assimp::Importer importer;
    return importer.IsExtensionSupported(path.substr(pos));
}
}  // namespace

bool AssimpLoader::isExtensionSupported(const std::string& path) const {
    return AssimpIsExtensionSupported(path);
}

}  // namespace Mesh
}  // namespace VNE
