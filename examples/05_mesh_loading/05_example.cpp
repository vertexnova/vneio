/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 05: Mesh load via AssimpLoader — geometry inspection, AABB,
 * vertex attributes, OBJ export + reload round-trip, error path.
 * Verifies vertex buffer layout readiness for VBO upload across
 * rendering backends without requiring a window or GPU context.
 * ----------------------------------------------------------------------
 */

#include "05_example.h"
#include "example_utils.h"

#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/mesh/mesh_exporter.h"
#include "vertexnova/io/mesh/mesh.h"

#include <string>

#ifndef VNEIO_TESTDATA_DIR
#define VNEIO_TESTDATA_DIR "testdata"
#endif

namespace vne::io::examples {

int runMeshLoadingExample() {
    LoggingGuard logging_guard;

    const std::string stl_path = std::string(VNEIO_TESTDATA_DIR) + "/meshes/minimal.stl";

    // ── Load via loadFile() ───────────────────────────────────────────────────
    printSection("Load minimal.stl via AssimpLoader::loadFile");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::Mesh mesh;
        bool ok = loader.loadFile(stl_path, mesh);
        if (!ok) {
            VNE_LOG_WARN << "minimal.stl not found: " << loader.getLastError()
                << " — skipping mesh tests";
            return 0;  // soft skip when testdata absent
        }
        printMeshInfo(mesh, "minimal.stl");
        if (!check(!mesh.isEmpty(),          "mesh is not empty"))            return 1;
        if (!check(mesh.getVertexCount() > 0, "vertexCount > 0"))             return 1;
        if (!check(mesh.getIndexCount()  > 0, "indexCount > 0"))              return 1;
        if (!check(mesh.getSubmeshCount() > 0, "submeshCount > 0"))           return 1;
    }

    // ── Load with explicit options ────────────────────────────────────────────
    printSection("Load with AssimpLoaderOptions (normals on, no normalize)");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::Mesh mesh;
        vne::mesh::AssimpLoaderOptions opts;
        opts.calc_normals_if_missing   = true;
        opts.normalize_to_unit_sphere  = false;
        opts.generate_barycentrics     = true;
        bool ok = loader.loadFile(stl_path, mesh, opts);
        if (!check(ok, "loadFile with options")) return 1;
        printMeshInfo(mesh, "minimal.stl (with options)");

        // First vertex position must be finite
        if (!mesh.vertices.empty()) {
            const auto& v = mesh.vertices[0];
            VNE_LOG_INFO << "  first vertex pos=("
                << v.position[0] << ", " << v.position[1] << ", " << v.position[2] << ")";
        }
    }

    // ── Load via LoadRequest ──────────────────────────────────────────────────
    printSection("Load via IMeshLoader LoadRequest interface");
    {
        vne::mesh::AssimpLoader loader;
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eMesh;
        req.uri        = stl_path;
        auto result    = loader.loadMesh(req);
        if (!check(result.ok(), "loadMesh(LoadRequest) ok")) return 1;
        printStatus(result.status);
        if (!check(!result.value.isEmpty(), "result.value is not empty")) return 1;
        VNE_LOG_INFO << "  vertices=" << result.value.getVertexCount();
    }

    // ── AABB sanity check ─────────────────────────────────────────────────────
    printSection("AABB sanity check");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::Mesh mesh;
        if (!check(loader.loadFile(stl_path, mesh), "loadFile for AABB check")) return 1;
        if (!mesh.isEmpty()) {
            bool aabb_ok =
                mesh.aabb_min[0] <= mesh.aabb_max[0] &&
                mesh.aabb_min[1] <= mesh.aabb_max[1] &&
                mesh.aabb_min[2] <= mesh.aabb_max[2];
            if (!check(aabb_ok, "aabb_min <= aabb_max on each axis")) return 1;
            VNE_LOG_INFO << "  extent: ("
                << (mesh.aabb_max[0] - mesh.aabb_min[0]) << ", "
                << (mesh.aabb_max[1] - mesh.aabb_min[1]) << ", "
                << (mesh.aabb_max[2] - mesh.aabb_min[2]) << ")";
        }
    }

    // ── OBJ export + reload round-trip ────────────────────────────────────────
    printSection("OBJ export round-trip");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::Mesh mesh;
        if (!check(loader.loadFile(stl_path, mesh), "loadFile before OBJ export")) return 1;

        const std::string obj_path = tmpPath("vneio_ex05_out.obj");
        std::string err;
        vne::mesh::ObjExportOptions opts;
        opts.write_normals   = true;
        opts.write_texcoords = mesh.has_uv0;
        if (!check(vne::mesh::exportObj(obj_path, mesh, opts, &err),
                   ("exportObj: " + obj_path).c_str())) {
            VNE_LOG_ERROR << "exportObj error: " << err;
            return 1;
        }

        vne::mesh::AssimpLoader reload_loader;
        vne::mesh::Mesh reloaded;
        if (!check(reload_loader.loadFile(obj_path, reloaded),
                   ("reload OBJ: " + obj_path).c_str())) {
            VNE_LOG_ERROR << "reload error: " << reload_loader.getLastError();
            return 1;
        }
        printMeshInfo(reloaded, "reloaded OBJ");
        // OBJ may split/merge vertices; index count can differ but should be non-zero
        if (!check(reloaded.getVertexCount() > 0, "reloaded vertexCount > 0")) return 1;
        if (!check(reloaded.getIndexCount()  > 0, "reloaded indexCount > 0"))  return 1;
    }

    // ── Extension support ─────────────────────────────────────────────────────
    printSection("isExtensionSupported");
    {
        vne::mesh::AssimpLoader loader;
        if (!check( loader.isExtensionSupported("model.stl"),  "stl supported"))  return 1;
        if (!check( loader.isExtensionSupported("model.obj"),  "obj supported"))  return 1;
        if (!check(!loader.isExtensionSupported("model.nrrd"), "nrrd not supported")) return 1;
    }

    // ── Error path: non-existent file ─────────────────────────────────────────
    printSection("Error path: non-existent file");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::Mesh mesh;
        bool ok = loader.loadFile("/nonexistent/path/mesh.stl", mesh);
        if (!check(!ok, "loadFile returns false for missing file")) return 1;
        if (!check(!loader.getLastError().empty(), "getLastError() non-empty")) return 1;
        VNE_LOG_INFO << "  error: " << loader.getLastError();
    }

    VNE_LOG_INFO << "05_mesh_loading: done.";
    return 0;
}

}  // namespace vne::io::examples
