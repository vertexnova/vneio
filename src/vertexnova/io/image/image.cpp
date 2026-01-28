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

#include <cstring>

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

    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            const int source_x = x * width_ / new_width;
            const int source_y = y * height_ / new_height;

            for (int c = 0; c < channels_; ++c) {
                const size_t dest_idx = static_cast<size_t>(y * new_width + x) * static_cast<size_t>(channels_) + static_cast<size_t>(c);
                const size_t src_idx = static_cast<size_t>(source_y * width_ + source_x) * static_cast<size_t>(channels_) + static_cast<size_t>(c);

                resized_data[dest_idx] = data_[src_idx];
            }
        }
    }

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

uint8_t* loadImage(const std::string& file_path, int* width, int* height, int* channels, int desired_channels, bool flip_vertically) {
    stbi_set_flip_vertically_on_load(flip_vertically);
    return stbi_load(file_path.c_str(), width, height, channels, desired_channels);
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
