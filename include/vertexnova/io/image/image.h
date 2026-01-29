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

#include <string>
#include <vector>
#include <cstdint>

namespace VNE {
namespace Image {

/**
 * @brief Image class for loading, manipulating and saving images
 *
 * This class provides a simple interface for working with images,
 * supporting various formats through stb_image. It works across
 * all supported platforms including desktop (Windows, Linux, macOS)
 * and mobile (Android, iOS) platforms.
 */
class Image {
   public:
    /**
     * @brief Default constructor creates an empty image
     */
    Image();

    /**
     * @brief Constructor that loads an image from a file
     * @param file_path Path to the image file
     */
    explicit Image(const std::string& file_path);

    /**
     * @brief Constructor that creates an image from raw data
     * @param data Raw pixel data
     * @param width Image width
     * @param height Image height
     * @param channels Number of color channels
     */
    Image(const uint8_t* data, int width, int height, int channels);

    /**
     * @brief Destructor
     */
    ~Image();

    /**
     * @brief Load an image from file
     * @param file_path Path to the image file
     * @param flip_vertically Whether to flip the image vertically after loading
     * (default = true)
     * @return True if loading succeeded, false otherwise
     */
    bool loadFromFile(const std::string& file_path, bool flip_vertically = true);

    /**
     * @brief Save the image to a file
     * @param file_path Path where the image will be saved
     * @param format Format to save as (jpg, png, bmp, tga)
     * @return True if saving succeeded, false otherwise
     */
    bool saveToFile(const std::string& file_path, const std::string& format = "png");

    /**
     * @brief Get the raw pixel data
     * @return Pointer to the pixel data
     */
    const uint8_t* getData() const;

    /**
     * @brief Get the width of the image
     * @return Image width in pixels
     */
    int getWidth() const;

    /**
     * @brief Get the height of the image
     * @return Image height in pixels
     */
    int getHeight() const;

    /**
     * @brief Get the number of color channels
     * @return Number of channels (1=grayscale, 3=RGB, 4=RGBA)
     */
    int getChannels() const;

    /**
     * @brief Resize the image
     * @param new_width New width in pixels
     * @param new_height New height in pixels
     * @return True if resizing succeeded, false otherwise
     */
    bool resize(int new_width, int new_height);

    /**
     * @brief Flip the image vertically
     */
    void flipVertically();

    /**
     * @brief Check if the image is empty
     * @return True if the image has no data
     */
    bool isEmpty() const;

   private:
    /**
     * @brief Clear the image data
     */
    void clear();

   private:
    std::vector<uint8_t> data_;  //!< Raw pixel data
    int width_;                  //!< Image width in pixels
    int height_;                 //!< Image height in pixels
    int channels_;               //!< Image channels (1=grayscale, 3=RGB, 4=RGBA)
};

/**
 * @brief Utility functions for image loading/saving
 */
namespace ImageUtils {
/**
 * @brief Load an image file directly into raw data
 * @param file_path Path to the image file
 * @param width Output parameter for image width
 * @param height Output parameter for image height
 * @param channels Output parameter for image channels
 * @param desired_channels Desired number of channels (0 = keep original)
 * @param flip_vertically Whether to flip the image vertically after loading
 * (default = true)
 * @return Pointer to image data, or nullptr if loading failed
 */
uint8_t* loadImage(const std::string& file_path,
                   int* width,
                   int* height,
                   int* channels,
                   int desired_channels = 0,
                   bool flip_vertically = true);

/**
 * @brief Free image data loaded by loadImage
 * @param data Pointer to the image data to free
 */
void freeImage(uint8_t* data);

/**
 * @brief Save raw image data to a file
 * @param file_path Path where the image will be saved
 * @param data Raw pixel data
 * @param width Image width
 * @param height Image height
 * @param channels Number of color channels
 * @param format Format to save as (jpg, png, bmp, tga)
 * @return True if saving succeeded, false otherwise
 */
bool saveImage(const std::string& file_path,
               const uint8_t* data,
               int width,
               int height,
               int channels,
               const std::string& format = "png");
}  // namespace ImageUtils

}  // namespace Image
}  // namespace VNE
