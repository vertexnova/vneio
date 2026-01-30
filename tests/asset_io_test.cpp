/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include <gtest/gtest.h>
#include "vertexnova/io/asset_io.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/utils/path_utils.h"
#include <filesystem>

using namespace vne::io;
using vne::io::utils::getTestdataPath;

TEST(AssetIOTest, LoadVolumeViaRegistry) {
    AssetIO io;
    io.registerVolumeLoader(std::make_unique<vne::Image::NrrdLoader>());
    io.registerVolumeLoader(std::make_unique<vne::Image::MhdLoader>());

    std::string path = getTestdataPath("volumes/small3d.nrrd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path;
    }

    LoadRequest request;
    request.asset_type = AssetType::eVolume;
    request.uri = path;

    LoadResult<vne::Image::Volume> result = io.loadVolume(request);
    ASSERT_TRUE(result.ok()) << result.status.message;
    EXPECT_FALSE(result.value.isEmpty());
    EXPECT_EQ(result.value.width(), 4);
    EXPECT_EQ(result.value.depth(), 4);
}

TEST(AssetIOTest, LoadImageViaRegistry) {
    AssetIO io;
    io.registerImageLoader(std::make_unique<vne::Image::StbImageLoader>());

    std::string path = getTestdataPath("textures/sample.png");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test image not found: " << path;
    }

    LoadRequest request;
    request.asset_type = AssetType::eImage;
    request.uri = path;

    LoadResult<vne::Image::Image> result = io.loadImage(request);
    ASSERT_TRUE(result.ok()) << result.status.message;
    EXPECT_FALSE(result.value.isEmpty());
}

TEST(AssetIOTest, LoadMeshViaRegistry) {
    AssetIO io;
    io.registerMeshLoader(std::make_unique<vne::Mesh::AssimpLoader>());

    std::string path = getTestdataPath("meshes/minimal.stl");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test mesh not found: " << path;
    }

    LoadRequest request;
    request.asset_type = AssetType::eMesh;
    request.uri = path;

    LoadResult<vne::Mesh::Mesh> result = io.loadMesh(request);
    ASSERT_TRUE(result.ok()) << result.status.message;
    EXPECT_FALSE(result.value.isEmpty());
}

TEST(AssetIOTest, NoLoaderReturnsError) {
    AssetIO io;
    LoadRequest request;
    request.asset_type = AssetType::eVolume;
    request.uri = "/nonexistent.nrrd";

    LoadResult<vne::Image::Volume> result = io.loadVolume(request);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.status.message.empty());
}
