/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 02: 2D image load / inspect / resize / save pipeline.
 * Exercises Image class and image_utils; verifies GPU-upload readiness
 * (non-empty, correct dims, channel count) without a renderer.
 * ----------------------------------------------------------------------
 */

#include "02_example.h"
#include "example_utils.h"

#include "vertexnova/io/image/image.h"

#include <string>

#ifndef VNEIO_TESTDATA_DIR
#define VNEIO_TESTDATA_DIR "testdata"
#endif

namespace vne::io::examples {

int runImageLoadingExample() {
    LoggingGuard logging_guard;

    const std::string tex_path = std::string(VNEIO_TESTDATA_DIR) + "/textures/sample.png";

    // ── Load via Image class ──────────────────────────────────────────────────
    printSection("Load image from file");
    {
        vne::image::Image img;
        bool ok = img.loadFromFile(tex_path);
        if (!check(ok, ("loadFromFile: " + tex_path).c_str())) {
            VNE_LOG_WARN << "testdata not found — skipping further image tests";
            return 0;  // soft skip when testdata absent
        }
        if (!check(!img.isEmpty(), "image is not empty")) {
            return 1;
        }
        if (!check(img.getWidth() > 0, "width > 0")) {
            return 1;
        }
        if (!check(img.getHeight() > 0, "height > 0")) {
            return 1;
        }
        if (!check(img.getChannels() >= 1 && img.getChannels() <= 4, "channels in [1,4]")) {
            return 1;
        }
        if (!check(img.getData() != nullptr, "getData() non-null")) {
            return 1;
        }

        VNE_LOG_INFO << "  width=" << img.getWidth() << "  height=" << img.getHeight()
                     << "  channels=" << img.getChannels()
                     << "  bytes=" << (img.getWidth() * img.getHeight() * img.getChannels());
    }

    // ── Constructor from file path ────────────────────────────────────────────
    printSection("Construct Image from file path");
    {
        vne::image::Image img(tex_path);
        if (!check(!img.isEmpty(), "Image(path) is not empty")) {
            return 1;
        }
    }

    // ── Resize + save + reload ────────────────────────────────────────────────
    printSection("Resize to 128x128 and save");
    {
        vne::image::Image img;
        if (!check(img.loadFromFile(tex_path), ("loadFromFile: " + tex_path).c_str())) {
            return 1;
        }

        if (!check(img.resize(128, 128), "resize(128,128) returned true")) {
            return 1;
        }
        if (!check(img.getWidth() == 128, "width == 128 after resize")) {
            return 1;
        }
        if (!check(img.getHeight() == 128, "height == 128 after resize")) {
            return 1;
        }

        const std::string out_path = tmpPath("vneio_ex02_resized.png");
        if (!check(img.saveToFile(out_path, "png"), ("saveToFile: " + out_path).c_str())) {
            return 1;
        }

        vne::image::Image reloaded;
        if (!check(reloaded.loadFromFile(out_path, false), "reload saved PNG")) {
            return 1;
        }
        if (!check(reloaded.getWidth() == 128, "reloaded width == 128")) {
            return 1;
        }
        if (!check(reloaded.getHeight() == 128, "reloaded height == 128")) {
            return 1;
        }
        VNE_LOG_INFO << "  reloaded channels=" << reloaded.getChannels();
    }

    // ── Flip vertically ───────────────────────────────────────────────────────
    printSection("Flip vertically");
    {
        vne::image::Image img;
        if (!check(img.loadFromFile(tex_path, false), ("loadFromFile (no flip): " + tex_path).c_str())) {
            return 1;
        }
        img.flipVertically();
        if (!check(!img.isEmpty(), "flipped image is not empty")) {
            return 1;
        }
    }

    // ── image_utils raw API ───────────────────────────────────────────────────
    printSection("image_utils raw load / free");
    {
        int w = 0, h = 0, c = 0;
        uint8_t* raw = vne::image::image_utils::loadImage(tex_path, &w, &h, &c);
        if (!check(raw != nullptr, "loadImage returned non-null")) {
            return 1;
        }
        if (!check(w > 0 && h > 0 && c > 0, "w/h/c all > 0")) {
            return 1;
        }
        VNE_LOG_INFO << "  raw: " << w << "x" << h << " ch=" << c;

        const std::string raw_out = tmpPath("vneio_ex02_raw.png");
        if (!check(vne::image::image_utils::saveImage(raw_out, raw, w, h, c, "png"),
                   ("image_utils::saveImage: " + raw_out).c_str())) {
            return 1;
        }

        vne::image::image_utils::freeImage(raw);
        if (!check(true, "freeImage (no crash)")) {
            return 1;
        }
    }

    // ── Construct from raw data ───────────────────────────────────────────────
    printSection("Construct Image from raw data buffer");
    {
        constexpr int kWidth = 4;
        constexpr int kHeight = 4;
        constexpr int kChannels = 4;
        uint8_t buf[kWidth * kHeight * kChannels] = {};
        for (int i = 0; i < kWidth * kHeight; ++i) {
            buf[i * kChannels + 0] = static_cast<uint8_t>((i % 2) * 255);
            buf[i * kChannels + 3] = 255;
        }
        vne::image::Image img(buf, kWidth, kHeight, kChannels);
        if (!check(!img.isEmpty(), "Image(raw) is not empty")) {
            return 1;
        }
        if (!check(img.getWidth() == kWidth, "width matches")) {
            return 1;
        }
        if (!check(img.getHeight() == kHeight, "height matches")) {
            return 1;
        }
        if (!check(img.getChannels() == kChannels, "channels matches")) {
            return 1;
        }
    }

    // ── Error path: non-existent file ─────────────────────────────────────────
    printSection("Error path: non-existent file");
    {
        vne::image::Image img;
        bool ok = img.loadFromFile("/nonexistent/path/image.png");
        if (!check(!ok, "loadFromFile returns false for missing file")) {
            return 1;
        }
        if (!check(img.isEmpty(), "isEmpty()==true after failed load")) {
            return 1;
        }
    }

    VNE_LOG_INFO << "02_image_loading: done.";
    return 0;
}

}  // namespace vne::io::examples
