#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * @file 02_example.h
 * @brief Example 02: 2D image load / inspect / resize / save pipeline.
 *
 * Uses testdata/textures/sample.png; also exercises image_utils.
 */

namespace vne::io::examples {

/** Run the image loading example; returns 0 on success, non-zero on failure. */
[[nodiscard]] int runImageLoadingExample();

}  // namespace vne::io::examples
