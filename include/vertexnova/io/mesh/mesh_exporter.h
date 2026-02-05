#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/mesh/mesh.h"

#include <string>

namespace vne::mesh {

/**
 * @file mesh_exporter.h
 * @brief Mesh export utilities.
 *
 * Current exporters:
 *  - OBJ (+MTL): widely supported, good for debug and interchange.
 *
 * Export is intentionally simple and deterministic. For advanced pipelines
 * (glTF, USD), integrate a dedicated library.
 */

/**
 * @struct ObjExportOptions
 * @brief Options for Wavefront OBJ (+MTL) export.
 */
struct ObjExportOptions {
    bool write_mtl = true;       //!< If true, write a .mtl material file.
    bool write_normals = true;   //!< If true, output vertex normals (vn).
    bool write_texcoords = true;//!< If true, output texture coordinates (vt).
    bool flip_v = false;         //!< If true, invert texcoord Y (e.g. for different renderer convention).
};

/**
 * @brief Export mesh to Wavefront OBJ (and optionally MTL).
 *
 * @param obj_path   Output path to .obj file.
 * @param mesh       Source mesh to export.
 * @param opts       Export options (default: write MTL, normals, texcoords).
 * @param out_error  If non-null, receives an error description on failure.
 * @return true if export succeeded, false otherwise (check out_error if provided).
 */
bool exportObj(const std::string& obj_path,
               const Mesh& mesh,
               const ObjExportOptions& opts = {},
               std::string* out_error = nullptr);

}  // namespace vne::mesh
