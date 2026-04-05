/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/volume_exporter.h"
#include "vertexnova/io/utils/path_utils.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace vne::image;
using vne::io::utils::getTestdataPath;

namespace {

constexpr float kNearEqualDefaultEpsilon = 5e-4f;

bool nearEqual(float a, float b, float eps = kNearEqualDefaultEpsilon) {
    return std::fabs(a - b) < eps;
}

void expectVolumeGeometryNear(const Volume& a, const Volume& b) {
    EXPECT_EQ(a.dims[0], b.dims[0]);
    EXPECT_EQ(a.dims[1], b.dims[1]);
    EXPECT_EQ(a.dims[2], b.dims[2]);
    EXPECT_EQ(a.pixel_type, b.pixel_type);
    EXPECT_EQ(a.components, b.components);
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(nearEqual(a.spacing[i], b.spacing[i])) << "spacing[" << i << "]";
        EXPECT_TRUE(nearEqual(a.origin[i], b.origin[i])) << "origin[" << i << "]";
    }
    for (int i = 0; i < 9; ++i) {
        EXPECT_TRUE(nearEqual(a.direction[i], b.direction[i])) << "direction[" << i << "]";
    }
    ASSERT_EQ(a.data.size(), b.data.size());
    EXPECT_EQ(0, std::memcmp(a.data.data(), b.data.data(), a.data.size()));
}

}  // namespace

TEST(VolumeTest, DefaultEmpty) {
    Volume vol;
    EXPECT_EQ(vol.width(), 0);
    EXPECT_EQ(vol.height(), 0);
    EXPECT_EQ(vol.depth(), 0);
    EXPECT_TRUE(vol.isEmpty());
    EXPECT_EQ(vol.voxelCount(), 0u);
    EXPECT_EQ(vol.byteCount(), 0u);
    EXPECT_FALSE(vol.hasExactBufferSize());
    EXPECT_TRUE(vol.hasScalarVoxels());
    EXPECT_TRUE(vol.hasIdentityDirection());
    EXPECT_FALSE(vol.isMetadataValid());
}

TEST(VolumeTest, BytesPerVoxel) {
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eUint8), 1);
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eUint16), 2);
    EXPECT_EQ(bytesPerVoxel(VolumePixelType::eFloat32), 4);
}

TEST(VolumeTest, VolumeHelpersFilledScalar) {
    Volume vol;
    vol.dims[0] = vol.dims[1] = vol.dims[2] = 2;
    vol.spacing[0] = vol.spacing[1] = vol.spacing[2] = 1.0f;
    vol.pixel_type = VolumePixelType::eUint8;
    vol.components = 1;
    vol.data.assign(8u, 0);
    for (int i = 0; i < 8; ++i) {
        vol.data[static_cast<size_t>(i)] = static_cast<uint8_t>(i);
    }
    EXPECT_TRUE(vol.hasExactBufferSize());
    EXPECT_TRUE(vol.hasScalarVoxels());
    EXPECT_TRUE(vol.hasIdentityDirection());
    EXPECT_TRUE(vol.isMetadataValid());
    EXPECT_EQ(vol.readVoxelAt<uint8_t>(7), 7);
}

TEST(VolumeTest, ReadVoxelAtOutOfRangeReturnsZero) {
    Volume vol;
    vol.dims[0] = vol.dims[1] = vol.dims[2] = 1;
    vol.pixel_type = VolumePixelType::eUint8;
    vol.components = 1;
    vol.data.assign(1u, 99);
    EXPECT_EQ(vol.readVoxelAt<uint8_t>(0), 99);
    EXPECT_EQ(vol.readVoxelAt<uint8_t>(1), 0);
    vol.pixel_type = VolumePixelType::eUint16;
    vol.data.assign(1u, 0);
    EXPECT_EQ(vol.readVoxelAt<uint16_t>(0), 0u);
}

TEST(VolumeTest, VolumeHelpersNonScalarComponents) {
    Volume vol;
    vol.dims[0] = vol.dims[1] = vol.dims[2] = 1;
    vol.pixel_type = VolumePixelType::eUint8;
    vol.components = 2;
    vol.data.assign(2u, 0);
    EXPECT_TRUE(vol.hasExactBufferSize());
    EXPECT_FALSE(vol.hasScalarVoxels());
    EXPECT_FALSE(vol.isMetadataValid());
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
    EXPECT_TRUE(vol.hasExactBufferSize());
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

TEST(VolumeTest, NrrdLoaderGeometrySynthetic) {
    NrrdLoader loader;
    Volume vol;
    const std::string path = "test_nrrd_geometry.nrrd";
    {
        std::ofstream f(path, std::ios::binary);
        ASSERT_TRUE(f);
        f << "NRRD0005\n";
        f << "type: uchar\n";
        f << "dimension: 3\n";
        f << "sizes: 2 2 2\n";
        f << "space dimension: 3\n";
        f << "space origin: (1,2,3)\n";
        f << "space directions: (2,0,0) (0,3,0) (0,0,4)\n";
        f << "encoding: raw\n";
        f << "endian: little\n";
        f << "\n";
        uint8_t data[8] = {0};
        f.write(reinterpret_cast<const char*>(data), 8);
    }
    ASSERT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_TRUE(vol.hasExactBufferSize());
    EXPECT_TRUE(nearEqual(vol.spacing[0], 2.0f));
    EXPECT_TRUE(nearEqual(vol.spacing[1], 3.0f));
    EXPECT_TRUE(nearEqual(vol.spacing[2], 4.0f));
    EXPECT_TRUE(nearEqual(vol.origin[0], 1.0f));
    EXPECT_TRUE(nearEqual(vol.origin[1], 2.0f));
    EXPECT_TRUE(nearEqual(vol.origin[2], 3.0f));
    EXPECT_TRUE(nearEqual(vol.direction[0], 1.0f));
    EXPECT_TRUE(nearEqual(vol.direction[4], 1.0f));
    EXPECT_TRUE(nearEqual(vol.direction[8], 1.0f));
    EXPECT_TRUE(vol.isMetadataValid());
    std::filesystem::remove(path);
}

TEST(VolumeTest, NrrdLoaderRejectDimension4) {
    NrrdLoader loader;
    Volume vol;
    const std::string path = "test_nrrd_dim4.nrrd";
    {
        std::ofstream f(path, std::ios::binary);
        ASSERT_TRUE(f);
        f << "NRRD0005\n";
        f << "type: uchar\n";
        f << "dimension: 4\n";
        f << "sizes: 2 2 2 2\n";
        f << "encoding: raw\n";
        f << "\n";
        char buf[16] = {};
        f.write(buf, 16);
    }
    EXPECT_FALSE(loader.load(path, vol));
    EXPECT_FALSE(loader.getLastError().empty());
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
        GTEST_SKIP() << "Test volume not found: " << path << " (run from project root with testdata/volumes present)";
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
        GTEST_SKIP() << "Test volume not found: " << path << " (run from project root with testdata/volumes present)";
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
    EXPECT_EQ(vol.voxelCount(),
              static_cast<size_t>(vol.width()) * static_cast<size_t>(vol.height()) * static_cast<size_t>(vol.depth()));
    EXPECT_EQ(vol.byteCount(), vol.voxelCount() * static_cast<size_t>(bytesPerVoxel(vol.pixel_type)));
}

TEST(VolumeTest, NrrdLoaderLoadSmall3dNrrd) {
    // 3D NRRD from testdata (small3d.nrrd), Teem-style: 4x4x4 uchar raw
    std::string path = getTestdataPath("volumes/small3d.nrrd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path << " (run from project root with testdata/volumes present)";
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

TEST(VolumeTest, MhdLoaderLoadSmall3dMhd) {
    // Detached MetaImage in testdata (small3d.mhd + small3d.raw), same 4x4x4 uchar ramp as small3d.nrrd
    std::string path = getTestdataPath("volumes/small3d.mhd");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path << " (run from project root with testdata/volumes present)";
    }

    MhdLoader loader;
    Volume vol;
    EXPECT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_FALSE(vol.isEmpty());
    EXPECT_EQ(vol.width(), 4);
    EXPECT_EQ(vol.height(), 4);
    EXPECT_EQ(vol.depth(), 4);
    EXPECT_EQ(vol.pixel_type, VolumePixelType::eUint8);
    EXPECT_EQ(vol.voxelCount(), 64u);
    EXPECT_EQ(vol.getData()[0], 0);
    EXPECT_EQ(vol.getData()[63], 63);
}

TEST(VolumeTest, MhdLoaderLoadSmall3dMha) {
    std::string path = getTestdataPath("volumes/small3d.mha");
    if (!std::filesystem::exists(path)) {
        GTEST_SKIP() << "Test volume not found: " << path << " (run from project root with testdata/volumes present)";
    }

    MhdLoader loader;
    Volume vol;
    EXPECT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_EQ(vol.width(), 4);
    EXPECT_EQ(vol.height(), 4);
    EXPECT_EQ(vol.depth(), 4);
    EXPECT_EQ(vol.getData()[0], 0);
    EXPECT_EQ(vol.getData()[63], 63);
}

TEST(VolumeTest, MhdLoaderDetachedMinimal) {
    MhdLoader loader;
    const std::string base = "test_mhd_detached";
    const std::string mhd_path = base + ".mhd";
    const std::string raw_path = base + ".raw";
    {
        std::ofstream mh(mhd_path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 2 2 2\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 1 1 1\n";
        mh << "ElementDataFile = " << base << ".raw\n";
        mh << "\n";
    }
    {
        std::ofstream rf(raw_path, std::ios::binary);
        ASSERT_TRUE(rf);
        uint8_t d[8] = {10, 11, 12, 13, 14, 15, 16, 17};
        rf.write(reinterpret_cast<const char*>(d), 8);
    }
    Volume vol;
    ASSERT_TRUE(loader.load(mhd_path, vol)) << loader.getLastError();
    EXPECT_TRUE(vol.hasExactBufferSize());
    EXPECT_EQ(vol.getData()[0], 10);
    EXPECT_EQ(vol.getData()[7], 17);
    std::filesystem::remove(mhd_path);
    std::filesystem::remove(raw_path);
}

TEST(VolumeTest, MhdLoaderMhaInline) {
    MhdLoader loader;
    const std::string path = "test_inline.mha";
    {
        std::ofstream f(path, std::ios::binary);
        ASSERT_TRUE(f);
        f << "ObjectType = Image\n";
        f << "NDims = 3\n";
        f << "DimSize = 1 1 1\n";
        f << "ElementType = MET_UCHAR\n";
        f << "ElementSpacing = 1 1 1\n";
        f << "ElementDataFile = LOCAL\n";
        f << "\n";
        uint8_t v = 42;
        f.write(reinterpret_cast<const char*>(&v), 1);
    }
    Volume vol;
    ASSERT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_TRUE(vol.hasExactBufferSize());
    EXPECT_EQ(vol.getData()[0], 42);
    std::filesystem::remove(path);
}

TEST(VolumeTest, MhdLoaderMsbUshortInline) {
    MhdLoader loader;
    const std::string path = "test_msb.mha";
    {
        std::ofstream f(path, std::ios::binary);
        ASSERT_TRUE(f);
        f << "ObjectType = Image\n";
        f << "NDims = 3\n";
        f << "DimSize = 1 1 1\n";
        f << "ElementType = MET_USHORT\n";
        f << "ElementSpacing = 1 1 1\n";
        f << "ElementByteOrderMSB = True\n";
        f << "ElementDataFile = LOCAL\n";
        f << "\n";
        uint8_t be[2] = {0x01, 0x02};
        f.write(reinterpret_cast<const char*>(be), 2);
    }
    Volume vol;
    ASSERT_TRUE(loader.load(path, vol)) << loader.getLastError();
    EXPECT_EQ(vol.pixel_type, VolumePixelType::eUint16);
    ASSERT_GE(vol.data.size(), sizeof(uint16_t));
    uint16_t value = 0;
    std::memcpy(&value, vol.getData(), sizeof(value));
    EXPECT_EQ(value, 258u);
    std::filesystem::remove(path);
}

TEST(VolumeTest, MhdLoaderPositionTransformMatrix) {
    MhdLoader loader;
    const std::string base = "test_mhd_meta";
    const std::string mhd_path = base + ".mhd";
    const std::string raw_path = base + ".raw";
    {
        std::ofstream mh(mhd_path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 1 1 1\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 2 2 2\n";
        mh << "Position = 10 20 30\n";
        mh << "TransformMatrix = 0 -1 0 1 0 0 0 0 1\n";
        mh << "ElementDataFile = " << base << ".raw\n";
        mh << "\n";
    }
    {
        std::ofstream rf(raw_path, std::ios::binary);
        ASSERT_TRUE(rf);
        uint8_t z = 9;
        rf.write(reinterpret_cast<const char*>(&z), 1);
    }
    Volume vol;
    ASSERT_TRUE(loader.load(mhd_path, vol)) << loader.getLastError();
    EXPECT_TRUE(nearEqual(vol.origin[0], 10.0f));
    EXPECT_TRUE(nearEqual(vol.origin[1], 20.0f));
    EXPECT_TRUE(nearEqual(vol.origin[2], 30.0f));
    EXPECT_TRUE(nearEqual(vol.direction[0], 0.0f));
    EXPECT_TRUE(nearEqual(vol.direction[1], -1.0f));
    EXPECT_TRUE(nearEqual(vol.direction[3], 1.0f));
    EXPECT_TRUE(nearEqual(vol.direction[4], 0.0f));
    EXPECT_TRUE(nearEqual(vol.direction[8], 1.0f));
    std::filesystem::remove(mhd_path);
    std::filesystem::remove(raw_path);
}

TEST(VolumeTest, MhdLoaderMissingRawFile) {
    MhdLoader loader;
    const std::string mhd_path = "test_mhd_missing_raw.mhd";
    {
        std::ofstream mh(mhd_path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 1 1 1\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 1 1 1\n";
        mh << "ElementDataFile = does_not_exist.raw\n";
    }
    Volume vol;
    EXPECT_FALSE(loader.load(mhd_path, vol));
    EXPECT_FALSE(loader.getLastError().empty());
    std::filesystem::remove(mhd_path);
}

TEST(VolumeTest, MhdLoaderShortRawFile) {
    MhdLoader loader;
    const std::string base = "test_mhd_short";
    const std::string mhd_path = base + ".mhd";
    const std::string raw_path = base + ".raw";
    {
        std::ofstream mh(mhd_path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 2 2 2\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 1 1 1\n";
        mh << "ElementDataFile = " << base << ".raw\n";
    }
    {
        std::ofstream rf(raw_path, std::ios::binary);
        ASSERT_TRUE(rf);
        uint8_t one = 1;
        rf.write(reinterpret_cast<const char*>(&one), 1);
    }
    Volume vol;
    EXPECT_FALSE(loader.load(mhd_path, vol));
    std::filesystem::remove(mhd_path);
    std::filesystem::remove(raw_path);
}

TEST(VolumeTest, MhdLoaderInvalidElementNumberOfChannels) {
    MhdLoader loader;
    const std::string path = "test_mhd_bad_channels.mhd";
    {
        std::ofstream mh(path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 1 1 1\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 1 1 1\n";
        mh << "ElementNumberOfChannels = not_an_int\n";
        mh << "ElementDataFile = LOCAL\n\n";
        mh << static_cast<char>(0);
    }
    Volume vol;
    EXPECT_FALSE(loader.load(path, vol));
    EXPECT_NE(loader.getLastError().find("ElementNumberOfChannels"), std::string::npos);
    std::filesystem::remove(path);
}

TEST(VolumeTest, MhdLoaderRejectMultiChannel) {
    MhdLoader loader;
    const std::string path = "test_mhd_channels.mhd";
    {
        std::ofstream mh(path);
        ASSERT_TRUE(mh);
        mh << "ObjectType = Image\n";
        mh << "NDims = 3\n";
        mh << "DimSize = 1 1 1\n";
        mh << "ElementType = MET_UCHAR\n";
        mh << "ElementSpacing = 1 1 1\n";
        mh << "ElementNumberOfChannels = 3\n";
        mh << "ElementDataFile = unused.raw\n";
    }
    Volume vol;
    EXPECT_FALSE(loader.load(path, vol));
    EXPECT_FALSE(loader.getLastError().empty());
    std::filesystem::remove(path);
}

TEST(VolumeTest, RoundTripNrrd) {
    Volume src;
    src.dims[0] = src.dims[1] = src.dims[2] = 2;
    src.spacing[0] = 0.5f;
    src.spacing[1] = 1.0f;
    src.spacing[2] = 2.0f;
    src.origin[0] = -1.0f;
    src.origin[1] = 0.25f;
    src.origin[2] = 3.0f;
    src.direction[0] = 0.0f;
    src.direction[1] = -1.0f;
    src.direction[2] = 0.0f;
    src.direction[3] = 1.0f;
    src.direction[4] = 0.0f;
    src.direction[5] = 0.0f;
    src.direction[6] = 0.0f;
    src.direction[7] = 0.0f;
    src.direction[8] = 1.0f;
    src.pixel_type = VolumePixelType::eUint8;
    src.components = 1;
    src.data.resize(8);
    for (int i = 0; i < 8; ++i) {
        src.data[static_cast<size_t>(i)] = static_cast<uint8_t>(i * 3);
    }
    const std::string path = "test_roundtrip.nrrd";
    std::string err;
    ASSERT_TRUE(exportNrrd(path, src, {}, &err)) << err;

    NrrdLoader loader;
    Volume dst;
    ASSERT_TRUE(loader.load(path, dst)) << loader.getLastError();
    expectVolumeGeometryNear(src, dst);
    std::filesystem::remove(path);
}

TEST(VolumeTest, RoundTripMhd) {
    Volume src;
    src.dims[0] = 2;
    src.dims[1] = 1;
    src.dims[2] = 2;
    src.spacing[0] = 1.5f;
    src.spacing[1] = 1.0f;
    src.spacing[2] = 0.25f;
    src.origin[0] = 5.0f;
    src.origin[1] = 6.0f;
    src.origin[2] = 7.0f;
    src.direction[0] = 1.0f;
    src.direction[1] = 0.0f;
    src.direction[2] = 0.0f;
    src.direction[3] = 0.0f;
    src.direction[4] = 0.0f;
    src.direction[5] = -1.0f;
    src.direction[6] = 0.0f;
    src.direction[7] = 1.0f;
    src.direction[8] = 0.0f;
    src.pixel_type = VolumePixelType::eUint8;
    src.components = 1;
    src.data.resize(4);
    src.data = {1, 2, 200, 255};
    const std::string mhd = "test_roundtrip.mhd";
    const std::string raw = "test_roundtrip.raw";
    std::string err;
    ASSERT_TRUE(exportMhd(mhd, src, {}, &err)) << err;

    MhdLoader loader;
    Volume dst;
    ASSERT_TRUE(loader.load(mhd, dst)) << loader.getLastError();
    expectVolumeGeometryNear(src, dst);
    std::filesystem::remove(mhd);
    std::filesystem::remove(raw);
}

TEST(VolumeTest, SyntheticRamp2x2x2) {
    Volume src;
    src.dims[0] = src.dims[1] = src.dims[2] = 2;
    src.pixel_type = VolumePixelType::eUint8;
    src.data.resize(8);
    for (int z = 0; z < 2; ++z) {
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                const int idx = x + y * 2 + z * 4;
                src.data[static_cast<size_t>(idx)] = static_cast<uint8_t>(idx);
            }
        }
    }
    const std::string path = "test_ramp.nrrd";
    std::string err;
    ASSERT_TRUE(exportNrrd(path, src, {}, &err)) << err;
    NrrdLoader loader;
    Volume dst;
    ASSERT_TRUE(loader.load(path, dst)) << loader.getLastError();
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(dst.data[static_cast<size_t>(i)], static_cast<uint8_t>(i)) << "voxel " << i;
    }
    EXPECT_TRUE(dst.isMetadataValid());
    std::filesystem::remove(path);
}
