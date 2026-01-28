/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/utils/path_utils.h"

#if !defined(VNEIO_TESTDATA_DIR)
#include <config.h>
#endif

namespace VNE {
namespace IO {
namespace Utils {

std::string getTestdataRoot() {
#ifdef VNEIO_TESTDATA_DIR
    return std::string(VNEIO_TESTDATA_DIR);
#else
    return std::string();
#endif
}

std::string getTestdataPath(const std::string& subpath) {
    std::string root = getTestdataRoot();
    if (root.empty()) return subpath;
    std::string p = subpath;
    while (!p.empty() && (p.front() == '/' || p.front() == '\\')) p.erase(0, 1);
    if (p.empty()) return root;
    if (root.back() == '/' || root.back() == '\\') return root + p;
    return root + "/" + p;
}

}  // namespace Utils
}  // namespace IO
}  // namespace VNE
