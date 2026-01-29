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

#include "vertexnova/io/image/image.h"

/* When using external/stb_image we link to the library; when using FetchContent stb we embed the impl */
#ifdef VNEIO_STB_HEADER_ONLY
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb_image.h"
#include "stb_image_write.h"

// Optional (better) resize. Enable by defining VNEIO_USE_STB_IMAGE_RESIZE.
// If your build uses stb as header-only, also enable impl below.
#if defined(VNEIO_USE_STB_IMAGE_RESIZE)
  #ifdef VNEIO_STB_HEADER_ONLY
    #define STB_IMAGE_RESIZE_IMPLEMENTATION
  #endif
  #include "stb_image_resize.h"
#endif

#include <algorithm>
#include <cstring>
#include <mutex>

namespace VNE {
namespace Image {

Image::Image()
    : width_(0)
    , height_(0)
    , channels_(0) {}

Image::Image(const std::string& file_path)
    : width_(0)
    , height_(0)
    , channels_(0) {
    loadFromFile(file_path);
}

Image::Image(const uint8_t* data, int width, int height, int channels)
    : width_(width)
    , height_(height)
    , channels_(channels) {
    if (width > 0 && height > 0 && channels > 0 && data) {
        const size_t data_size = static_cast<size_t>(width * height * channels);
        data_.resize(data_size);
        std::memcpy(data_.data(), data, data_size);
    }
}

Image::~Image() {
    clear();
}

bool Image::loadFromFile(const std::string& file_path, bool flip_vertically) {
    clear();

    int width;
    int height;
    int channels;
    uint8_t* data = ImageUtils::loadImage(file_path, &width, &height, &channels, 0, flip_vertically);

    if (!data) {
        return false;
    }

    width_ = width;
    height_ = height;
    channels_ = channels;

    size_t data_size = static_cast<size_t>(width_ * height_ * channels_);
    data_.resize(data_size);
    std::memcpy(data_.data(), data, data_size);

    ImageUtils::freeImage(data);
    return true;
}

bool Image::saveToFile(const std::string& file_path, const std::string& format) {
    if (isEmpty()) {
        return false;
    }

    return ImageUtils::saveImage(file_path, data_.data(), width_, height_, channels_, format);
}

const uint8_t* Image::getData() const {
    return isEmpty() ? nullptr : data_.data();
}

int Image::getWidth() const {
    return width_;
}

int Image::getHeight() const {
    return height_;
}

int Image::getChannels() const {
    return channels_;
}

bool Image::resize(int new_width, int new_height) {
    if (isEmpty() || new_width <= 0 || new_height <= 0) {
        return false;
    }

    const size_t new_size = static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(channels_);
    std::vector<uint8_t> resized_data(new_size);

    // Prefer stb_image_resize if enabled; otherwise use a simple bilinear fallback.
#if defined(VNEIO_USE_STB_IMAGE_RESIZE)
    const int ok = stbir_resize_uint8(
        data_.data(), width_, height_, 0,
        resized_data.data(), new_width, new_height, 0,
        channels_);
    if (ok == 0) return false;
#else
    // Bilinear resize (uint8). Good enough when stb_image_resize isn't available.
    const float sx = static_cast<float>(width_) / static_cast<float>(new_width);
    const float sy = static_cast<float>(height_) / static_cast<float>(new_height);
    for (int y = 0; y < new_height; ++y) {
        const float fy = (static_cast<float>(y) + 0.5f) * sy - 0.5f;
        const int y0 = std::max(0, std::min(height_ - 1, static_cast<int>(fy)));
        const int y1 = std::max(0, std::min(height_ - 1, y0 + 1));
        const float ty = std::min(1.0f, std::max(0.0f, fy - static_cast<float>(y0)));
        for (int x = 0; x < new_width; ++x) {
            const float fx = (static_cast<float>(x) + 0.5f) * sx - 0.5f;
            const int x0 = std::max(0, std::min(width_ - 1, static_cast<int>(fx)));
            const int x1 = std::max(0, std::min(width_ - 1, x0 + 1));
            const float tx = std::min(1.0f, std::max(0.0f, fx - static_cast<float>(x0)));

            const uint8_t* p00 = data_.data() + (static_cast<size_t>(y0) * static_cast<size_t>(width_) + static_cast<size_t>(x0)) * static_cast<size_t>(channels_);
            const uint8_t* p10 = data_.data() + (static_cast<size_t>(y0) * static_cast<size_t>(width_) + static_cast<size_t>(x1)) * static_cast<size_t>(channels_);
            const uint8_t* p01 = data_.data() + (static_cast<size_t>(y1) * static_cast<size_t>(width_) + static_cast<size_t>(x0)) * static_cast<size_t>(channels_);
            const uint8_t* p11 = data_.data() + (static_cast<size_t>(y1) * static_cast<size_t>(width_) + static_cast<size_t>(x1)) * static_cast<size_t>(channels_);

            uint8_t* dst = resized_data.data() + (static_cast<size_t>(y) * static_cast<size_t>(new_width) + static_cast<size_t>(x)) * static_cast<size_t>(channels_);
            for (int c = 0; c < channels_; ++c) {
                const float v00 = static_cast<float>(p00[c]);
                const float v10 = static_cast<float>(p10[c]);
                const float v01 = static_cast<float>(p01[c]);
                const float v11 = static_cast<float>(p11[c]);
                const float v0 = v00 + (v10 - v00) * tx;
                const float v1 = v01 + (v11 - v01) * tx;
                const float v = v0 + (v1 - v0) * ty;
                dst[c] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, v)));
            }
        }
    }
#endif

    data_ = std::move(resized_data);
    width_ = new_width;
    height_ = new_height;
    return true;
}

void Image::flipVertically() {
    if (isEmpty()) {
        return;
    }

    const size_t stride = static_cast<size_t>(width_) * static_cast<size_t>(channels_);
    std::vector<uint8_t> row_buffer(stride);

    for (int y = 0; y < height_ / 2; ++y) {
        const size_t top_offset = static_cast<size_t>(y) * stride;
        const size_t bottom_offset = static_cast<size_t>(height_ - 1 - y) * stride;

        uint8_t* top_row = data_.data() + top_offset;
        uint8_t* bottom_row = data_.data() + bottom_offset;

        std::memcpy(row_buffer.data(), top_row, stride);
        std::memcpy(top_row, bottom_row, stride);
        std::memcpy(bottom_row, row_buffer.data(), stride);
    }
}

bool Image::isEmpty() const {
    return data_.empty() || width_ <= 0 || height_ <= 0 || channels_ <= 0;
}

void Image::clear() {
    data_.clear();
    width_ = 0;
    height_ = 0;
    channels_ = 0;
}

namespace ImageUtils {

namespace {
// stb_image uses global state (e.g. stbi_set_flip_vertically_on_load).
// Guard it for thread-safe asset loading.
std::mutex g_stbi_mutex;
}

uint8_t* loadImage(const std::string& file_path, int* width, int* height, int* channels, int desired_channels, bool flip_vertically) {
    std::lock_guard<std::mutex> lock(g_stbi_mutex);
    stbi_set_flip_vertically_on_load(flip_vertically);
    uint8_t* data = stbi_load(file_path.c_str(), width, height, channels, desired_channels);
    // Reset to default (false) so callers that don't request flipping are unaffected.
    stbi_set_flip_vertically_on_load(false);
    return data;
}

void freeImage(uint8_t* data) {
    if (data) {
        stbi_image_free(data);
    }
}

bool saveImage(const std::string& file_path, const uint8_t* data, int width, int height, int channels, const std::string& format) {
    if (!data || width <= 0 || height <= 0 || (channels != 1 && channels != 3 && channels != 4)) {
        return false;
    }

    int result = 0;
    const int stride = width * channels;

    if (format == "png") {
        result = stbi_write_png(file_path.c_str(), width, height, channels, data, stride);
    } else if (format == "jpg" || format == "jpeg") {
        result = stbi_write_jpg(file_path.c_str(), width, height, channels, data, 90);
    } else if (format == "bmp") {
        result = stbi_write_bmp(file_path.c_str(), width, height, channels, data);
    } else if (format == "tga") {
        result = stbi_write_tga(file_path.c_str(), width, height, channels, data);
    } else {
        result = stbi_write_png(file_path.c_str(), width, height, channels, data, stride);
    }

    return result != 0;
}

}  // namespace ImageUtils

}  // namespace Image
}  // namespace VNE
