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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/utils/path_utils.h"

#include <filesystem>
#include <cstring>

using namespace vne::image;
using vne::io::utils::getTestdataPath;

namespace {
const std::string kTestImagePath = getTestdataPath("textures/sample.png");
const std::string kNonExistentPath = getTestdataPath("textures/does_not_exist.png");

const std::string kTestOutputDir = "test_output";
}  // namespace

class ImageTest : public ::testing::Test {
   protected:
    void SetUp() override {
        if (!std::filesystem::exists(kTestImagePath)) {
            GTEST_SKIP() << "Test image not found: " << kTestImagePath << " (run from project root)";
        }
    }

    void TearDown() override {
        for (const auto& p : temp_files_to_delete_) {
            std::error_code ec;
            std::filesystem::remove(p, ec);
        }
    }

    static std::string getTestImagePath() { return std::filesystem::exists(kTestImagePath) ? kTestImagePath : ""; }

    static std::vector<uint8_t> createTestData(int width, int height, int channels) {
        std::vector<uint8_t> data(static_cast<size_t>(width * height * channels));
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<uint8_t>(i % 256);
        }
        return data;
    }

   protected:
    std::vector<std::string> temp_files_to_delete_;
};

TEST_F(ImageTest, DefaultConstructor) {
    Image image;
    EXPECT_TRUE(image.isEmpty());
    EXPECT_EQ(image.getWidth(), 0);
    EXPECT_EQ(image.getHeight(), 0);
    EXPECT_EQ(image.getChannels(), 0);
    EXPECT_EQ(image.getData(), nullptr);
}

TEST_F(ImageTest, FileConstructor) {
    std::string path = getTestImagePath();
    if (path.empty())
        return;

    Image valid_image(path);
    EXPECT_FALSE(valid_image.isEmpty());
    EXPECT_GT(valid_image.getWidth(), 0);
    EXPECT_GT(valid_image.getHeight(), 0);
    EXPECT_NE(valid_image.getData(), nullptr);

    Image non_existent(kNonExistentPath);
    EXPECT_TRUE(non_existent.isEmpty());
}

TEST_F(ImageTest, DataConstructor) {
    const int width = 10;
    const int height = 10;
    const int channels = 4;
    auto test_data = createTestData(width, height, channels);

    Image image(test_data.data(), width, height, channels);
    EXPECT_FALSE(image.isEmpty());
    EXPECT_EQ(image.getWidth(), width);
    EXPECT_EQ(image.getHeight(), height);
    EXPECT_EQ(image.getChannels(), channels);
    ASSERT_NE(image.getData(), nullptr);
    for (int i = 0; i < width * height * channels; ++i) {
        EXPECT_EQ(image.getData()[i], test_data[i]) << "Mismatch at index " << i;
    }

    Image invalid(test_data.data(), 0, 0, 0);
    EXPECT_TRUE(invalid.isEmpty());

    Image null_img(nullptr, width, height, channels);
    EXPECT_TRUE(null_img.isEmpty());
}

TEST_F(ImageTest, LoadFromFile) {
    std::string path = getTestImagePath();
    if (path.empty())
        return;

    Image image;
    EXPECT_TRUE(image.loadFromFile(path));
    EXPECT_FALSE(image.isEmpty());
    EXPECT_GT(image.getWidth(), 0);
    EXPECT_GT(image.getHeight(), 0);
    EXPECT_NE(image.getData(), nullptr);

    Image no_file;
    EXPECT_FALSE(no_file.loadFromFile(kNonExistentPath));
    EXPECT_TRUE(no_file.isEmpty());
}

TEST_F(ImageTest, SaveToFile) {
    std::string path = getTestImagePath();
    if (path.empty())
        return;

    Image image(path);
    ASSERT_FALSE(image.isEmpty());

    std::filesystem::create_directories(kTestOutputDir);
    std::vector<std::string> formats = {"png", "jpg", "bmp", "tga"};
    for (const auto& format : formats) {
        std::string out = kTestOutputDir + "/test_save." + format;
        temp_files_to_delete_.push_back(out);
        EXPECT_TRUE(image.saveToFile(out, format));
        EXPECT_TRUE(std::filesystem::exists(out));

        Image loaded(out);
        EXPECT_FALSE(loaded.isEmpty());
        EXPECT_EQ(loaded.getWidth(), image.getWidth());
        EXPECT_EQ(loaded.getHeight(), image.getHeight());
    }

    Image empty;
    EXPECT_FALSE(empty.saveToFile(kTestOutputDir + "/empty.png"));
}

TEST_F(ImageTest, Resize) {
    std::string path = getTestImagePath();
    if (path.empty())
        return;

    Image image(path);
    ASSERT_FALSE(image.isEmpty());

    EXPECT_TRUE(image.resize(32, 32));
    EXPECT_EQ(image.getWidth(), 32);
    EXPECT_EQ(image.getHeight(), 32);

    EXPECT_TRUE(image.resize(128, 128));
    EXPECT_EQ(image.getWidth(), 128);
    EXPECT_EQ(image.getHeight(), 128);

    Image empty_img;
    EXPECT_FALSE(empty_img.resize(32, 32));

    EXPECT_FALSE(image.resize(0, 0));
    EXPECT_EQ(image.getWidth(), 128);
    EXPECT_EQ(image.getHeight(), 128);
}

TEST_F(ImageTest, FlipVertically) {
    const int width = 4;
    const int height = 4;
    const int channels = 3;
    std::vector<uint8_t> data(width * height * channels);
    for (int i = 0; i < width * channels; ++i)
        data[i] = 255;
    for (int i = (height - 1) * width * channels; i < height * width * channels; ++i)
        data[i] = 0;

    Image image(data.data(), width, height, channels);
    ASSERT_FALSE(image.isEmpty());
    EXPECT_EQ(image.getData()[0], 255);

    image.flipVertically();
    for (int i = 0; i < width * channels; ++i) {
        EXPECT_EQ(image.getData()[i], 0) << "Top row at " << i;
    }
    for (int i = (height - 1) * width * channels; i < height * width * channels; ++i) {
        EXPECT_EQ(image.getData()[i], 255) << "Bottom row at " << i;
    }

    Image empty;
    empty.flipVertically();
    EXPECT_TRUE(empty.isEmpty());
}

TEST_F(ImageTest, ImageUtils) {
    std::string path = getTestImagePath();
    if (path.empty())
        return;

    int width = 0;
    int height = 0;
    int channels = 0;
    uint8_t* data = image_utils::loadImage(path, &width, &height, &channels);
    ASSERT_NE(data, nullptr);
    EXPECT_GT(width, 0);
    EXPECT_GT(height, 0);
    EXPECT_GE(channels, 1);
    image_utils::freeImage(data);

    const int tw = 16;
    const int th = 16;
    const int tc = 4;
    std::vector<uint8_t> test_data = createTestData(tw, th, tc);
    std::filesystem::create_directories(kTestOutputDir);
    std::string save_path = kTestOutputDir + "/test_direct_save.png";
    temp_files_to_delete_.push_back(save_path);

    EXPECT_TRUE(image_utils::saveImage(save_path, test_data.data(), tw, th, tc));
    EXPECT_TRUE(std::filesystem::exists(save_path));

    EXPECT_FALSE(image_utils::saveImage(save_path, nullptr, tw, th, tc));
    EXPECT_FALSE(image_utils::saveImage(save_path, test_data.data(), 0, th, tc));
    EXPECT_FALSE(image_utils::saveImage(save_path, test_data.data(), tw, 0, tc));
}
