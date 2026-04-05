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
 * @file 07_example.cpp
 * @brief Example 07: Load-time performance benchmark.
 *
 * All assets are generated synthetically and written to /tmp so the benchmark is fully self-contained
 * (no testdata directory required).
 *
 * Sections:
 * - Volume NRRD load throughput (64³ uint16, N=10)
 * - Volume MHD/MHA load throughput (64³ uint16, N=10)
 * - Image PNG load throughput (512×512 RGBA, N=10)
 * - Mesh OBJ load — baseline (100×100 grid, N=5)
 * - Mesh OBJ load — post-process (same grid + normalize + barycentrics)
 * - AssetIO registry dispatch overhead (volume, N=10)
 */

#include "07_example.h"
#include "example_utils.h"

#include "vertexnova/io/asset_io.h"
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/image/volume_exporter.h"
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/mesh/mesh_exporter.h"
#include "vertexnova/io/mesh/mesh.h"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

namespace vne::io::examples {

// ── Benchmark parameters ──────────────────────────────────────────────────────

static constexpr int kVolumeReps = 10;
static constexpr int kImageReps = 10;
static constexpr int kMeshReps = 5;

// Volume: 64×64×64 uint16 — 512 KB
static constexpr int kVolDim = 64;
static constexpr int kImgWidth = 512;
static constexpr int kImgHeight = 512;
static constexpr int kImgChannels = 4;  // RGBA
static constexpr int kMeshGrid = 100;   // 100×100 quads → ~10k verts, ~20k triangles

// ── Synthetic data builders ───────────────────────────────────────────────────

static vne::image::Volume makeSyntheticVolume() {
    vne::image::Volume v;
    v.dims[0] = v.dims[1] = v.dims[2] = kVolDim;
    v.pixel_type = vne::image::VolumePixelType::eUint16;
    v.components = 1;
    v.spacing[0] = v.spacing[1] = v.spacing[2] = 1.0f;
    v.origin[0] = v.origin[1] = v.origin[2] = 0.0f;
    v.direction[0] = v.direction[4] = v.direction[8] = 1.0f;  // identity
    v.data.resize(v.byteCount());
    auto* p = reinterpret_cast<uint16_t*>(v.data.data());
    for (std::size_t i = 0; i < v.voxelCount(); ++i) {
        p[i] = static_cast<uint16_t>(i & 0xFFFF);
    }
    return v;
}

// Build a flat N×N quad grid exported to OBJ so Assimp can load it.
static vne::mesh::Mesh makeSyntheticMesh(int n = kMeshGrid) {
    vne::mesh::Mesh mesh;
    const int vcnt = (n + 1) * (n + 1);
    mesh.vertices.resize(static_cast<std::size_t>(vcnt));

    int idx = 0;
    for (int y = 0; y <= n; ++y) {
        for (int x = 0; x <= n; ++x) {
            auto& v = mesh.vertices[static_cast<std::size_t>(idx++)];
            v.position[0] = static_cast<float>(x);
            v.position[1] = static_cast<float>(y);
            v.position[2] = 0.0f;
            v.normal[0] = 0.0f;
            v.normal[1] = 0.0f;
            v.normal[2] = 1.0f;
            v.texcoord0[0] = static_cast<float>(x) / static_cast<float>(n);
            v.texcoord0[1] = static_cast<float>(y) / static_cast<float>(n);
        }
    }

    mesh.indices.reserve(static_cast<std::size_t>(n * n * 6));
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            auto tl = static_cast<uint32_t>(y * (n + 1) + x);
            auto tr = tl + 1u;
            auto bl = tl + static_cast<uint32_t>(n + 1);
            auto br = bl + 1u;
            mesh.indices.push_back(tl);
            mesh.indices.push_back(bl);
            mesh.indices.push_back(br);
            mesh.indices.push_back(tl);
            mesh.indices.push_back(br);
            mesh.indices.push_back(tr);
        }
    }

    vne::mesh::Submesh sub;
    sub.first_index = 0;
    sub.index_count = static_cast<uint32_t>(mesh.indices.size());
    sub.material_index = 0;
    mesh.parts.push_back(sub);
    mesh.has_normals = true;
    mesh.has_uv0 = true;
    mesh.aabb_min[0] = mesh.aabb_min[1] = mesh.aabb_min[2] = 0.0f;
    mesh.aabb_max[0] = mesh.aabb_max[1] = static_cast<float>(n);
    mesh.aabb_max[2] = 0.0f;
    return mesh;
}

// ── Main entry point ──────────────────────────────────────────────────────────

int runPerformanceExample() {
    LoggingGuard logging_guard;

    VNE_LOG_INFO << "07_performance: all assets are synthetic (no testdata required)";
    VNE_LOG_INFO << "  volume: " << kVolDim << "^3 uint16 = " << (kVolDim * kVolDim * kVolDim * 2 / 1024) << " KB";
    VNE_LOG_INFO << "  image:  " << kImgWidth << "x" << kImgHeight
                 << " RGBA = " << (kImgWidth * kImgHeight * kImgChannels / 1024) << " KB";
    VNE_LOG_INFO << "  mesh:   " << kMeshGrid << "x" << kMeshGrid << " grid = " << ((kMeshGrid + 1) * (kMeshGrid + 1))
                 << " verts  " << (kMeshGrid * kMeshGrid * 2) << " triangles";

    // ── Prepare synthetic volume files ────────────────────────────────────────
    const vne::image::Volume src_vol = makeSyntheticVolume();
    const std::string nrrd_path = tmpPath("vneio_bench_vol.nrrd");
    const std::string mha_path = tmpPath("vneio_bench_vol.mha");
    {
        std::string err;
        vne::image::NrrdExportOptions nrrd_opts;
        if (!check(vne::image::exportNrrd(nrrd_path, src_vol, nrrd_opts, &err), "export synthetic volume as .nrrd")) {
            VNE_LOG_ERROR << err;
            return 1;
        }
        vne::image::MhdExportOptions mhd_opts;
        mhd_opts.inline_data = true;
        if (!check(vne::image::exportMhd(mha_path, src_vol, mhd_opts, &err), "export synthetic volume as .mha")) {
            VNE_LOG_ERROR << err;
            return 1;
        }
    }
    const std::size_t vol_bytes = src_vol.byteCount();

    // ── Prepare synthetic PNG ─────────────────────────────────────────────────
    const std::string png_path = tmpPath("vneio_bench_img.png");
    {
        // Build raw RGBA buffer: gradient pattern
        const std::size_t stride = static_cast<std::size_t>(kImgWidth * kImgChannels);
        std::vector<uint8_t> pixels(static_cast<std::size_t>(kImgHeight) * stride);
        for (int y = 0; y < kImgHeight; ++y) {
            for (int x = 0; x < kImgWidth; ++x) {
                uint8_t* px =
                    pixels.data() + static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * kImgChannels;
                px[0] = static_cast<uint8_t>(x & 0xFF);
                px[1] = static_cast<uint8_t>(y & 0xFF);
                px[2] = 128;
                px[3] = 255;
            }
        }
        vne::image::Image img(pixels.data(), kImgWidth, kImgHeight, kImgChannels);
        if (!check(img.saveToFile(png_path), ("save synthetic PNG: " + png_path).c_str())) {
            VNE_LOG_WARN << "PNG save failed — skipping image benchmark (stb_image write may be unavailable)";
        }
    }

    // ── Prepare synthetic OBJ ─────────────────────────────────────────────────
    const std::string obj_path = tmpPath("vneio_bench_mesh.obj");
    {
        const vne::mesh::Mesh src_mesh = makeSyntheticMesh();
        std::string err;
        vne::mesh::ObjExportOptions obj_opts;
        obj_opts.write_normals = true;
        obj_opts.write_texcoords = true;
        obj_opts.write_mtl = false;
        if (!check(vne::mesh::exportObj(obj_path, src_mesh, obj_opts, &err), "export synthetic mesh as .obj")) {
            VNE_LOG_ERROR << err;
            return 1;
        }
        VNE_LOG_INFO << "  OBJ written: " << src_mesh.getVertexCount() << " verts  " << src_mesh.getIndexCount() / 3
                     << " triangles";
    }

    // ── Section 1: Volume NRRD ────────────────────────────────────────────────
    printSection("Benchmark 1: NrrdLoader (N=" + std::to_string(kVolumeReps) + ")");
    {
        vne::image::NrrdLoader loader;
        auto times = timeN(kVolumeReps, [&] {
            vne::image::Volume v;
            static_cast<void>(loader.load(nrrd_path, v));
        });
        reportBench("NrrdLoader", times, vol_bytes);
    }

    // ── Section 2: Volume MHD/MHA ─────────────────────────────────────────────
    printSection("Benchmark 2: MhdLoader inline .mha (N=" + std::to_string(kVolumeReps) + ")");
    {
        vne::image::MhdLoader loader;
        auto times = timeN(kVolumeReps, [&] {
            vne::image::Volume v;
            static_cast<void>(loader.load(mha_path, v));
        });
        reportBench("MhdLoader (.mha)", times, vol_bytes);
    }

    // ── Section 3: Image PNG ──────────────────────────────────────────────────
    printSection("Benchmark 3: StbImageLoader PNG (N=" + std::to_string(kImageReps) + ")");
    {
        vne::image::StbImageLoader loader;
        const std::size_t img_bytes = static_cast<std::size_t>(kImgWidth * kImgHeight * kImgChannels);

        // Verify PNG was written before timing
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eImage;
        req.uri = png_path;
        auto probe = loader.loadImage(req);
        if (!probe.ok()) {
            VNE_LOG_WARN << "PNG not available (" << errorCodeName(probe.status.code) << ") — skipping image benchmark";
        } else {
            auto times = timeN(kImageReps, [&] {
                vne::image::Image img;
                static_cast<void>(img.loadFromFile(png_path));
            });
            reportBench("StbImageLoader (PNG)", times, img_bytes);
        }
    }

    // ── Section 4: Mesh baseline ──────────────────────────────────────────────
    printSection("Benchmark 4: AssimpLoader OBJ — default options (N=" + std::to_string(kMeshReps) + ")");
    std::size_t mesh_vertex_count = 0;
    std::size_t mesh_triangle_count = 0;
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::AssimpLoaderOptions opts;
        // defaults: flip_uvs=true, gen_tangents=true, triangulate=true, pre_transform=true
        // normalize and barycentrics both off
        opts.normalize_to_unit_sphere = false;
        opts.generate_barycentrics = false;

        auto times = timeN(kMeshReps, [&] {
            vne::mesh::Mesh m;
            static_cast<void>(loader.loadFile(obj_path, m, opts));
            mesh_vertex_count = m.getVertexCount();
            mesh_triangle_count = m.getIndexCount() / 3;
        });
        VNE_LOG_INFO << "  vertices=" << mesh_vertex_count << "  triangles=" << mesh_triangle_count;
        reportBench("AssimpLoader OBJ (default)", times);
    }

    // ── Section 5: Mesh with expensive post-processing ────────────────────────
    printSection("Benchmark 5: AssimpLoader OBJ — normalize + barycentrics (N=" + std::to_string(kMeshReps) + ")");
    {
        vne::mesh::AssimpLoader loader;
        vne::mesh::AssimpLoaderOptions opts;
        opts.normalize_to_unit_sphere = true;
        opts.generate_barycentrics = true;

        std::size_t expanded_verts = 0;
        auto times = timeN(kMeshReps, [&] {
            vne::mesh::Mesh m;
            static_cast<void>(loader.loadFile(obj_path, m, opts));
            expanded_verts = m.getVertexCount();
        });
        VNE_LOG_INFO << "  vertices after barycentrics=" << expanded_verts
                     << "  (expected ~3x vs baseline: " << mesh_vertex_count * 3 << ")";
        reportBench("AssimpLoader OBJ (normalize+barycentrics)", times);
    }

    // ── Section 6: AssetIO registry dispatch overhead ─────────────────────────
    printSection("Benchmark 6: AssetIO registry dispatch vs direct load (N=" + std::to_string(kVolumeReps) + ")");
    {
        // Direct load
        vne::image::NrrdLoader direct_loader;
        auto direct_times = timeN(kVolumeReps, [&] {
            vne::image::Volume v;
            static_cast<void>(direct_loader.load(nrrd_path, v));
        });

        // Registry load
        vne::io::AssetIO registry;
        registry.registerVolumeLoader(std::make_unique<vne::image::NrrdLoader>());
        vne::io::LoadRequest req;
        req.asset_type = vne::io::AssetType::eVolume;
        req.uri = nrrd_path;
        auto registry_times = timeN(kVolumeReps, [&] { registry.loadVolume(req); });

        reportBench("NrrdLoader (direct)", direct_times, vol_bytes);
        reportBench("AssetIO registry (NRRD)", registry_times, vol_bytes);

        double direct_avg = std::accumulate(direct_times.begin(), direct_times.end(), 0.0) / direct_times.size();
        double registry_avg =
            std::accumulate(registry_times.begin(), registry_times.end(), 0.0) / registry_times.size();
        double overhead_pct = (registry_avg - direct_avg) / direct_avg * 100.0;
        VNE_LOG_INFO << "  registry dispatch overhead: " << std::fixed << std::setprecision(1) << overhead_pct << "%"
                     << "  (expected ~0 — dominated by I/O)";
    }

    VNE_LOG_INFO << "07_performance: done.";
    return 0;
}

}  // namespace vne::io::examples
