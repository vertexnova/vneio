#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Shared header-only helpers for VneIo headless examples.
 * Pulls in @ref logging_guard.h (RAII + VertexNova logging when
 * VNEIO_EXAMPLES_HAS_LOGGING is set, or console @c VNE_LOG_* stubs).
 * Use @c VNE_LOG_INFO << …, @c VNE_LOG_WARN << …, @c VNE_LOG_ERROR << …
 * in example sources and in these helpers.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/common/status.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/mesh/mesh.h"

#include "logging_guard.h"

#include <sstream>
#include <string>

namespace vne::io::examples {

// ── Section banner ────────────────────────────────────────────────────────────

inline void printSection(const char* title) {
    VNE_LOG_INFO << "=== " << title << " ===";
}

// ── Status ────────────────────────────────────────────────────────────────────

inline const char* errorCodeName(ErrorCode code) {
    switch (code) {
        case ErrorCode::eOk:
            return "eOk";
        case ErrorCode::eUnknown:
            return "eUnknown";
        case ErrorCode::eInvalidArgument:
            return "eInvalidArgument";
        case ErrorCode::eNotImplemented:
            return "eNotImplemented";
        case ErrorCode::eOutOfMemory:
            return "eOutOfMemory";
        case ErrorCode::eFileNotFound:
            return "eFileNotFound";
        case ErrorCode::eFileOpenFailed:
            return "eFileOpenFailed";
        case ErrorCode::eFileReadFailed:
            return "eFileReadFailed";
        case ErrorCode::eFileWriteFailed:
            return "eFileWriteFailed";
        case ErrorCode::ePathInvalid:
            return "ePathInvalid";
        case ErrorCode::eUnsupportedFormat:
            return "eUnsupportedFormat";
        case ErrorCode::eUnsupportedFeature:
            return "eUnsupportedFeature";
        case ErrorCode::eParseError:
            return "eParseError";
        case ErrorCode::eDataCorrupt:
            return "eDataCorrupt";
        case ErrorCode::eDataTruncated:
            return "eDataTruncated";
        case ErrorCode::eInvalidDimensions:
            return "eInvalidDimensions";
        case ErrorCode::eInvalidPixelType:
            return "eInvalidPixelType";
        case ErrorCode::eThirdPartyError:
            return "eThirdPartyError";
    }
    return "?";
}

inline void printStatus(const Status& s) {
    if (s.ok()) {
        VNE_LOG_INFO << "  status: OK";
    } else {
        std::ostringstream oss;
        oss << "  status: " << errorCodeName(s.code);
        if (!s.subsystem.empty()) {
            oss << "  subsystem: " << s.subsystem;
        }
        if (!s.message.empty()) {
            oss << "  msg: " << s.message;
        }
        if (!s.path.empty()) {
            oss << "  path: " << s.path;
        }
        VNE_LOG_WARN << oss.str();
    }
}

// ── Volume info ───────────────────────────────────────────────────────────────

inline const char* pixelTypeName(vne::image::VolumePixelType t) {
    using T = vne::image::VolumePixelType;
    switch (t) {
        case T::eUint8:
            return "uint8";
        case T::eInt8:
            return "int8";
        case T::eUint16:
            return "uint16";
        case T::eInt16:
            return "int16";
        case T::eUint32:
            return "uint32";
        case T::eInt32:
            return "int32";
        case T::eFloat32:
            return "float32";
        case T::eFloat64:
            return "float64";
        case T::eUnknown:
            return "unknown";
    }
    return "?";
}

inline void printVolumeInfo(const vne::image::Volume& v, const char* label = "Volume") {
    VNE_LOG_INFO << label << ":";
    VNE_LOG_INFO << "  dims:    " << v.dims[0] << " x " << v.dims[1] << " x " << v.dims[2];
    VNE_LOG_INFO << "  spacing: " << v.spacing[0] << ", " << v.spacing[1] << ", " << v.spacing[2];
    VNE_LOG_INFO << "  origin:  " << v.origin[0] << ", " << v.origin[1] << ", " << v.origin[2];
    VNE_LOG_INFO << "  dir:     [" << v.direction[0] << ", " << v.direction[1] << ", " << v.direction[2] << " | "
                 << v.direction[3] << ", " << v.direction[4] << ", " << v.direction[5] << " | " << v.direction[6]
                 << ", " << v.direction[7] << ", " << v.direction[8] << "]";
    VNE_LOG_INFO << "  type:    " << pixelTypeName(v.pixel_type) << "  components=" << v.components
                 << "  bytes=" << v.byteCount();
    VNE_LOG_INFO << "  valid:   " << (v.isMetadataValid() ? "yes" : "no")
                 << "  identity_dir: " << (v.hasIdentityDirection() ? "yes" : "no")
                 << "  scalar: " << (v.hasScalarVoxels() ? "yes" : "no");
}

// ── Mesh info ─────────────────────────────────────────────────────────────────

inline void printMeshInfo(const vne::mesh::Mesh& m, const char* label = "Mesh") {
    VNE_LOG_INFO << label << ":";
    VNE_LOG_INFO << "  vertices: " << m.getVertexCount() << "  indices: " << m.getIndexCount()
                 << "  submeshes: " << m.getSubmeshCount() << "  materials: " << m.getMaterialCount();
    VNE_LOG_INFO << "  aabb_min: (" << m.aabb_min[0] << ", " << m.aabb_min[1] << ", " << m.aabb_min[2] << ")";
    VNE_LOG_INFO << "  aabb_max: (" << m.aabb_max[0] << ", " << m.aabb_max[1] << ", " << m.aabb_max[2] << ")";
    VNE_LOG_INFO << "  normals: " << (m.has_normals ? "yes" : "no") << "  tangents: " << (m.has_tangent ? "yes" : "no")
                 << "  uv0: " << (m.has_uv0 ? "yes" : "no");
}

// ── Temp path ─────────────────────────────────────────────────────────────────

inline std::string tmpPath(const std::string& filename) {
#if defined(_WIN32)
    char buf[512] = {};
    GetTempPathA(static_cast<DWORD>(sizeof(buf)), buf);
    return std::string(buf) + filename;
#else
    return std::string("/tmp/") + filename;
#endif
}

// ── Simple assertion ──────────────────────────────────────────────────────────

inline bool check(bool condition, const char* description) {
    if (condition) {
        VNE_LOG_INFO << "[PASS] " << description;
    } else {
        VNE_LOG_ERROR << "[FAIL] " << description;
    }
    return condition;
}

}  // namespace vne::io::examples
