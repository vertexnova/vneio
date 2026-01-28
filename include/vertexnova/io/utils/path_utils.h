#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Path utilities for testdata and project paths (requires config.h when built with CMake).
 * ----------------------------------------------------------------------
 */

#include <string>

namespace VNE {
namespace IO {
namespace Utils {

/**
 * @brief Returns the absolute path of the VneIo testdata resources root.
 *
 * When built with CMake, this is the directory containing "meshes", "textures", "materials", etc.
 * (typically testdata/vneresources/resources). Tests use getTestdataRoot() + "/meshes/teapot.stl"
 * or getTestdataRoot() + "/textures/container.jpg".
 *
 * Include directory for the build must contain the generated config.h (VNEIO_TESTDATA_DIR).
 * @return Testdata resources root path, or empty string if not defined.
 */
std::string getTestdataRoot();

}  // namespace Utils
}  // namespace IO
}  // namespace VNE
