/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include <gtest/gtest.h>
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/utils/path_utils.h"
#include <filesystem>
#include <fstream>
#include <cstdint>

using namespace VNE::Image;
using VNE::IO::Utils::getTestdataPath;

TEST(VolumeTest, DefaultEmpty) {
    Volume vol;
    EXPECT_EQ(vol.width(), 0);
    EXPECT_EQ(vol.height(), 0);
    EXPECT_EQ(vol.depth(), 0);
    EXPECT_TRUE(vol.isEmpty());
    EXPECT_EQ(vol.voxelCount(), 0u);
    EXPECT_EQ(vol.byteCount(), 0u);
}

TEST(VolumeTest, BytesPerVoxel) {
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eUint8), 1);
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eUint16), 2);
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eFloat32), 4);
}

TEST(VolumeTest, NrrdLoaderExtensionSupport) {
    NrrdLoader loader;
    EXPECT_TRUE(loader.isExtensionSupported("test.nrrd"));
    EXPECT_TRUE(loader.isExtensionSupported("x.nhdr"));
    EXPECT_FALSE(loader.isExtensionSupported("x.raw"));
    EXPECT_FALSE(loader.isExtensionSupported("x.mhd"));
}

TEST(VolumeTest, MhdLoaderExtensionSupport) {
    MhdLoader loader;
    EXPECT_TRUE(loader.isExtensionSupported("test.mhd"));
    EXPECT_TRUE(loader.isExtensionSupported("x.mha"));
    EXPECT_FALSE(loader.isExtensionSupported("x.nrrd"));
}

TEST(VolumeTest, NrrdLoaderLoadMinimalSynthetic) {
    NrrdLoader loader;
    Volume vol;

    std::string path = "test_minimal.nrrd";
    {
        std::ofstream f(path, std::ios::binary);
        ASSERT_TRUE(f);
        f << "NRRD0005\n";
        f << "type: uchar\n";
        f << "dimension: 3\n";
        f << "sizes: 2 2 2\n";
        f << "encoding: raw\n";
        f << "\n";
        uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        f.write(reinterpret_cast<const char*>(data), 8);
    }

    EXPECT_TRUE(loader.load(path, vol));
    EXPECT_FALSE(vol.isEmpty());
    EXPECT_EQ(vol.width(), 2);
    EXPECT_EQ(vol.height(), 2);
    EXPECT_EQ(vol.depth(), 2);
    EXPECT_EQ(vol.pixel_type, VolumePixelType::eUint8);
    EXPECT_EQ(vol.voxelCount(), 8u);
    EXPECT_EQ(vol.data.size(), 8u);
    EXPECT_EQ(vol.getData()[0], 0);
    EXPECT_EQ(vol.getData()[7], 7);

    std::filesystem::remove(path);
}

TEST(VolumeTest, NrrdLoaderLoadNonexistent) {
    NrrdLoader loader;
    Volume vol;
    EXPECT_FALSE(loader.load("/nonexistent/path.nrrd", vol));
    EXPECT_TRUE(vol.isEmpty());
    EXPECT_FALSE(loader.getLastError().empty());
}

TEST(VolumeTest, NrrdLoaderLoadTestdataVolume) {
    // Use a small 1D NRRD from testdata (loader supports 1D, 2D, 3D)
    std::string path = getTestdataPath("volumes/an-hist.nrrd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path
                     << " (run from project root with testdata/volumes present)";
    }

    NrrdLoader loader;
    Volume vol;
    EXPECT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_FALSE(vol.isEmpty());
    EXPECT_GT(vol.width(), 0);
    EXPECT_GT(vol.height(), 0);
    EXPECT_GT(vol.depth(), 0);
    EXPECT_GT(vol.voxelCount(), 0u);
    EXPECT_GT(vol.byteCount(), 0u);
    EXPECT_NE(vol.getData(), nullptr);
}

TEST(VolumeTest, NrrdLoaderLoadFoolNrrd) {
    // 2D NRRD from testdata (fool.nrrd), loaded as 3D with depth padded to 1
    std::string path = getTestdataPath("volumes/fool.nrrd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path
                     << " (run from project root with testdata/volumes present)";
    }

    NrrdLoader loader;
    Volume vol;
    EXPECT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_FALSE(vol.isEmpty());
    EXPECT_GT(vol.width(), 0);
    EXPECT_GT(vol.height(), 0);
    EXPECT_EQ(vol.depth(), 1) << "fool.nrrd is 2D, depth should be padded to 1";
    EXPECT_GT(vol.voxelCount(), 0u);
    EXPECT_GT(vol.byteCount(), 0u);
    EXPECT_NE(vol.getData(), nullptr);
    EXPECT_EQ(vol.voxelCount(), static_cast<size_t>(vol.width()) * static_cast<size_t>(vol.height())
                                      * static_cast<size_t>(vol.depth()));
    EXPECT_EQ(vol.byteCount(), vol.voxelCount() * static_cast<size_t>(bytesPerVoxel(vol.pixel_type)));
}

TEST(VolumeTest, NrrdLoaderLoadSmall3dNrrd) {
    // 3D NRRD from testdata (small3d.nrrd), Teem-style: 4x4x4 uchar raw
    std::string path = getTestdataPath("volumes/small3d.nrrd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path
                     << " (run from project root with testdata/volumes present)";
    }

    NrrdLoader loader;
    Volume vol;
    EXPECT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_FALSE(vol.isEmpty());
    EXPECT_EQ(vol.width(), 4);
    EXPECT_EQ(vol.height(), 4);
    EXPECT_EQ(vol.depth(), 4);
    EXPECT_EQ(vol.pixel_type, VolumePixelType::eUint8);
    EXPECT_EQ(vol.voxelCount(), 64u);
    EXPECT_EQ(vol.byteCount(), 64u);
    EXPECT_NE(vol.getData(), nullptr);
    EXPECT_EQ(vol.getData()[0], 0);
    EXPECT_EQ(vol.getData()[63], 63);
}
