/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * OBJ exporter.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/mesh/mesh_exporter.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace VNE::Mesh {

namespace {

std::string Dirname(const std::string& p) {
    std::filesystem::path pp(p);
    return pp.parent_path().string();
}

std::string Stem(const std::string& p) {
    std::filesystem::path pp(p);
    return pp.stem().string();
}

void SetError(std::string* out_error, const std::string& msg) {
    if (out_error) *out_error = msg;
}

}  // namespace

bool ExportObj(const std::string& obj_path, const Mesh& mesh, const ObjExportOptions& opts, std::string* out_error) {
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        SetError(out_error, "ExportObj: mesh is empty");
        return false;
    }

    std::ofstream f(obj_path, std::ios::out | std::ios::trunc);
    if (!f) {
        SetError(out_error, "ExportObj: cannot open output file");
        return false;
    }

    const std::string base = Stem(obj_path);
    const std::string mtl_name = base + ".mtl";

    f << "# Exported by vneio\n";
    f << "o " << (mesh.name.empty() ? base : mesh.name) << "\n";
    if (opts.write_mtl) {
        f << "mtllib " << mtl_name << "\n";
    }

    // Write vertex attributes
    for (const auto& v : mesh.vertices) {
        f << "v " << v.position[0] << " " << v.position[1] << " " << v.position[2] << "\n";
    }

    const bool write_vt = opts.write_texcoords && mesh.has_uv0;
    if (write_vt) {
        for (const auto& v : mesh.vertices) {
            const float u = v.texcoord0[0];
            const float vv = opts.flip_v ? (1.0f - v.texcoord0[1]) : v.texcoord0[1];
            f << "vt " << u << " " << vv << "\n";
        }
    }

    const bool write_vn = opts.write_normals && mesh.has_normals;
    if (write_vn) {
        for (const auto& v : mesh.vertices) {
            f << "vn " << v.normal[0] << " " << v.normal[1] << " " << v.normal[2] << "\n";
        }
    }

    // Faces (OBJ is 1-based indexing)
    auto write_face = [&](uint32_t i0, uint32_t i1, uint32_t i2) {
        const uint32_t a = i0 + 1;
        const uint32_t b = i1 + 1;
        const uint32_t c = i2 + 1;

        if (write_vt && write_vn) {
            f << "f "
              << a << "/" << a << "/" << a << " "
              << b << "/" << b << "/" << b << " "
              << c << "/" << c << "/" << c << "\n";
        } else if (write_vt && !write_vn) {
            f << "f "
              << a << "/" << a << " "
              << b << "/" << b << " "
              << c << "/" << c << "\n";
        } else if (!write_vt && write_vn) {
            f << "f "
              << a << "//" << a << " "
              << b << "//" << b << " "
              << c << "//" << c << "\n";
        } else {
            f << "f " << a << " " << b << " " << c << "\n";
        }
    };

    auto material_name = [&](uint32_t idx) -> std::string {
        if (idx < mesh.materials.size() && !mesh.materials[idx].name.empty()) {
            return mesh.materials[idx].name;
        }
        std::ostringstream oss;
        oss << "mat_" << idx;
        return oss.str();
    };

    if (mesh.parts.empty()) {
        if (opts.write_mtl) {
            f << "usemtl " << material_name(0) << "\n";
        }
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            write_face(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
        }
    } else {
        for (const auto& part : mesh.parts) {
            if (opts.write_mtl) {
                f << "usemtl " << material_name(part.material_index) << "\n";
            }
            const uint32_t end = part.first_index + part.index_count;
            for (uint32_t i = part.first_index; i + 2 < end; i += 3) {
                write_face(mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]);
            }
        }
    }

    if (!f) {
        SetError(out_error, "ExportObj: failed while writing");
        return false;
    }

    // MTL file
    if (opts.write_mtl) {
        const std::string mtl_path = (Dirname(obj_path).empty() ? mtl_name : (Dirname(obj_path) + "/" + mtl_name));
        std::ofstream m(mtl_path, std::ios::out | std::ios::trunc);
        if (!m) {
            SetError(out_error, "ExportObj: cannot open MTL output file");
            return false;
        }
        m << "# Exported by vneio\n";
        for (uint32_t i = 0; i < static_cast<uint32_t>(mesh.materials.size()); ++i) {
            const auto& mat = mesh.materials[i];
            const std::string n = material_name(i);
            m << "newmtl " << n << "\n";
            m << "Ka 0 0 0\n";
            m << "Kd " << mat.base_color[0] << " " << mat.base_color[1] << " " << mat.base_color[2] << "\n";
            m << "d " << mat.base_color[3] << "\n";
            if (!mat.base_color_tex.empty()) {
                m << "map_Kd " << mat.base_color_tex << "\n";
            }
            m << "\n";
        }
        if (!m) {
            SetError(out_error, "ExportObj: failed while writing MTL");
            return false;
        }
    }

    return true;
}

}  // namespace VNE::Mesh
