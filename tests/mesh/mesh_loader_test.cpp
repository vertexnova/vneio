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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/utils/path_utils.h"

#include <filesystem>

using namespace VNE::Mesh;
using VNE::IO::Utils::getTestdataPath;

namespace {
const std::string kTeapotPath = getTestdataPath("meshes/teapot.stl");
const std::string kNonExistentPath = getTestdataPath("meshes/does_not_exist.stl");
const std::string kInvalidMeshPath = getTestdataPath("meshes/invalid_mesh.stl");
}  // namespace

class MeshLoaderTest : public ::testing::Test {
   protected:
    void SetUp() override {
        if (!std::filesystem::exists(kTeapotPath)) {
            GTEST_SKIP() << "Test mesh not found: " << kTeapotPath
                         << " (run from project root with testdata/vneresources updated)";
        }
    }
    void TearDown() override {}
};

TEST_F(MeshLoaderTest, DefaultConstructor) {
    Mesh mesh;
    EXPECT_TRUE(mesh.isEmpty());
    EXPECT_EQ(mesh.getVertexCount(), 0u);
    EXPECT_EQ(mesh.getIndexCount(), 0u);
    EXPECT_EQ(mesh.getSubmeshCount(), 0u);
    EXPECT_EQ(mesh.getMaterialCount(), 0u);
    EXPECT_TRUE(mesh.vertices.empty());
    EXPECT_TRUE(mesh.indices.empty());
    EXPECT_TRUE(mesh.parts.empty());
    EXPECT_TRUE(mesh.materials.empty());
}

TEST_F(MeshLoaderTest, AssimpLoaderDefaultConstructor) {
    AssimpLoader loader;
    EXPECT_TRUE(loader.getLastError().empty());
}

TEST_F(MeshLoaderTest, LoadTeapotSTL) {
    AssimpLoader loader;
    Mesh mesh;

    AssimpLoaderOptions opts;
    opts.triangulate = true;
    opts.calc_normals_if_missing = true;
    opts.pre_transform_vertices = false;
    opts.flip_uvs = false;
    opts.gen_tangents = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh, opts));
    EXPECT_FALSE(mesh.isEmpty());

    EXPECT_GT(mesh.getVertexCount(), 0u);
    EXPECT_GT(mesh.getIndexCount(), 0u);
    EXPECT_GT(mesh.getSubmeshCount(), 0u);
    EXPECT_FALSE(mesh.vertices.empty());
    EXPECT_FALSE(mesh.indices.empty());
    EXPECT_FALSE(mesh.parts.empty());

    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        EXPECT_LT(mesh.indices[i], mesh.getVertexCount());
    }
    EXPECT_EQ(mesh.getIndexCount() % 3, 0u);

    for (const auto& part : mesh.parts) {
        EXPECT_LE(part.first_index + part.index_count, mesh.getIndexCount());
        EXPECT_GT(part.index_count, 0u);
        EXPECT_EQ(part.index_count % 3, 0u);
    }
}

TEST_F(MeshLoaderTest, LoadWithOptions) {
    AssimpLoader loader;
    Mesh mesh;

    AssimpLoaderOptions safe_opts;
    safe_opts.triangulate = true;
    safe_opts.calc_normals_if_missing = true;
    safe_opts.pre_transform_vertices = false;
    safe_opts.flip_uvs = false;
    safe_opts.gen_tangents = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh, safe_opts));
    EXPECT_FALSE(mesh.isEmpty());

    Mesh mesh_custom;
    AssimpLoaderOptions custom_opts;
    custom_opts.flip_uvs = false;
    custom_opts.gen_tangents = false;
    custom_opts.triangulate = true;
    custom_opts.calc_normals_if_missing = true;
    custom_opts.pre_transform_vertices = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh_custom, custom_opts));
    EXPECT_FALSE(mesh_custom.isEmpty());
    EXPECT_GT(mesh_custom.getVertexCount(), 0u);
    EXPECT_GT(mesh_custom.getIndexCount(), 0u);
    EXPECT_GT(mesh_custom.getSubmeshCount(), 0u);
}

TEST_F(MeshLoaderTest, LoadNonExistentFile) {
    AssimpLoader loader;
    Mesh mesh;

    EXPECT_FALSE(loader.loadFile(kNonExistentPath, mesh));
    EXPECT_TRUE(mesh.isEmpty());
    EXPECT_FALSE(loader.getLastError().empty());
}

TEST_F(MeshLoaderTest, FormatSupport) {
    AssimpLoader::isExtensionSupported("test.stl");
    AssimpLoader::isExtensionSupported("test.obj");
    AssimpLoader::isExtensionSupported("test.fbx");
    AssimpLoader::isExtensionSupported("test.gltf");
    EXPECT_FALSE(AssimpLoader::isExtensionSupported("test.xyz"));
    EXPECT_FALSE(AssimpLoader::isExtensionSupported("test.unknown"));
}

TEST_F(MeshLoaderTest, VertexAttributes) {
    AssimpLoader loader;
    Mesh mesh;

    AssimpLoaderOptions opts;
    opts.triangulate = true;
    opts.calc_normals_if_missing = true;
    opts.pre_transform_vertices = false;
    opts.flip_uvs = false;
    opts.gen_tangents = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh, opts));
    EXPECT_FALSE(mesh.isEmpty());

    for (const auto& vertex : mesh.vertices) {
        float pos_magnitude =
            vertex.position[0] * vertex.position[0] + vertex.position[1] * vertex.position[1]
            + vertex.position[2] * vertex.position[2];
        EXPECT_GT(pos_magnitude, 0.0f);

        float normal_magnitude = vertex.normal[0] * vertex.normal[0] + vertex.normal[1] * vertex.normal[1]
                                 + vertex.normal[2] * vertex.normal[2];
        EXPECT_NEAR(normal_magnitude, 1.0f, 0.01f);

        EXPECT_GE(vertex.texcoord0[0], 0.0f);
        EXPECT_LE(vertex.texcoord0[0], 1.0f);
        EXPECT_GE(vertex.texcoord0[1], 0.0f);
        EXPECT_LE(vertex.texcoord0[1], 1.0f);
    }
}

TEST_F(MeshLoaderTest, MeshProperties) {
    AssimpLoader loader;
    Mesh mesh;

    AssimpLoaderOptions opts;
    opts.triangulate = true;
    opts.calc_normals_if_missing = true;
    opts.pre_transform_vertices = false;
    opts.flip_uvs = false;
    opts.gen_tangents = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh, opts));
    EXPECT_FALSE(mesh.isEmpty());

    EXPECT_EQ(mesh.name, kTeapotPath);
    EXPECT_GE(mesh.getMaterialCount(), 0u);

    for (const auto& part : mesh.parts) {
        if (mesh.getMaterialCount() > 0) {
            EXPECT_LT(part.material_index, mesh.getMaterialCount());
        }
    }
}

TEST_F(MeshLoaderTest, ErrorHandling) {
    AssimpLoader loader;
    Mesh mesh;

    EXPECT_FALSE(loader.loadFile("", mesh));
    EXPECT_TRUE(mesh.isEmpty());
    EXPECT_FALSE(loader.getLastError().empty());

    if (std::filesystem::exists(kInvalidMeshPath)) {
        EXPECT_FALSE(loader.loadFile(kInvalidMeshPath, mesh));
        EXPECT_TRUE(mesh.isEmpty());
        EXPECT_FALSE(loader.getLastError().empty());
    }
}

TEST_F(MeshLoaderTest, MultipleLoads) {
    AssimpLoader loader;
    Mesh mesh1, mesh2;

    AssimpLoaderOptions opts;
    opts.triangulate = true;
    opts.calc_normals_if_missing = true;
    opts.pre_transform_vertices = false;
    opts.flip_uvs = false;
    opts.gen_tangents = false;

    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh1, opts));
    EXPECT_TRUE(loader.loadFile(kTeapotPath, mesh2, opts));

    EXPECT_FALSE(mesh1.isEmpty());
    EXPECT_FALSE(mesh2.isEmpty());
    EXPECT_EQ(mesh1.getVertexCount(), mesh2.getVertexCount());
    EXPECT_EQ(mesh1.getIndexCount(), mesh2.getIndexCount());
    EXPECT_EQ(mesh1.getSubmeshCount(), mesh2.getSubmeshCount());
}
