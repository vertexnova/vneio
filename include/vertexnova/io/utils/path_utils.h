#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include <string>

namespace vne {
namespace io {
namespace utils {

/**
 * @brief Returns the absolute path of the VneIo testdata resources root.
 *
 * When built with CMake, this is the testdata root (typically testdata/). Tests use
 * getTestdataPath("meshes/minimal.stl") or getTestdataPath("textures/sample.png").
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

}  // namespace utils
}  // namespace io
}  // namespace vne
