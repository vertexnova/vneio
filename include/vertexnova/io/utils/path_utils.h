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
 * (typically testdata/vneresources/resources). Tests use getTestdataPath("meshes/teapot.stl")
 * or getTestdataPath("textures/container.jpg").
 *
 * Include directory for the build must contain the generated config.h (VNEIO_TESTDATA_DIR).
 * @return Testdata resources root path, or empty string if not defined.
 */
std::string getTestdataRoot();

/**
 * @brief Returns the absolute path to a file or subdirectory under the testdata resources root.
 *
 * Equivalent to getTestdataRoot() + "/" + subpath, with leading slashes in subpath trimmed.
 * @param subpath Relative path under testdata resources (e.g. "meshes/teapot.stl", "textures/container.jpg").
 * @return Full path, or subpath unchanged if getTestdataRoot() is empty.
 */
std::string getTestdataPath(const std::string& subpath);

}  // namespace Utils
}  // namespace IO
}  // namespace VNE
