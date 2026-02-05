#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Mesh export utilities.
 *
 * Current exporters:
 *  - OBJ (+MTL) : widely supported, good for debug and interchange
 *
 * Notes:
 *  - Export is intentionally simple and deterministic.
 *  - For advanced pipelines (glTF, USD), integrate a dedicated library.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/mesh/mesh.h"

#include <string>

namespace vne::mesh {

struct ObjExportOptions {
    bool write_mtl = true;
    bool write_normals = true;
    bool write_texcoords = true;
    bool flip_v = false;  // invert texcoord Y (handy when your renderer uses a different convention)
};

/**
 * @brief Export mesh to Wavefront OBJ (and optionally MTL).
 *
 * @param obj_path Output path to .obj
 * @param mesh     Source mesh
 * @param opts     Export options
 * @param out_error If non-null, receives error description on failure
 */
bool exportObj(const std::string& obj_path,
               const Mesh& mesh,
               const ObjExportOptions& opts = {},
               std::string* out_error = nullptr);

}  // namespace vne::mesh
