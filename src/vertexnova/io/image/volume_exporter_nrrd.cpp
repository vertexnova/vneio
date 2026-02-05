/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * NRRD exporter (raw encoding only).
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume_exporter.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#include "vertexnova/io/common/binary_io.h"

namespace vne::image {

namespace {

void setError(std::string* out_error, const std::string& msg) {
    if (out_error) {
        *out_error = msg;
    }
}

std::string pixelTypeToNrrd(VolumePixelType t) {
    switch (t) {
        case VolumePixelType::eUint8:
            return "uint8";
        case VolumePixelType::eInt8:
            return "int8";
        case VolumePixelType::eUint16:
            return "uint16";
        case VolumePixelType::eInt16:
            return "int16";
        case VolumePixelType::eUint32:
            return "uint32";
        case VolumePixelType::eInt32:
            return "int32";
        case VolumePixelType::eFloat32:
            return "float";
        case VolumePixelType::eFloat64:
            return "double";
        case VolumePixelType::eUnknown:
            break;
    }
    return "unknown";
}

std::string dirname(const std::string& p) {
    std::filesystem::path pp(p);
    return pp.parent_path().string();
}

}  // namespace

bool exportNrrd(const std::string& nrrd_or_nhdr_path,
               const Volume& vol,
               const NrrdExportOptions& opts,
               std::string* out_error) {
    if (vol.isEmpty()) {
        setError(out_error, "exportNrrd: volume is empty");
        return false;
    }
    if (vol.components != 1) {
        setError(out_error, "exportNrrd: only scalar volumes (components==1) are supported");
        return false;
    }
    const std::string type = pixelTypeToNrrd(vol.pixel_type);
    if (type == "unknown") {
        setError(out_error, "exportNrrd: unsupported pixel type");
        return false;
    }

    const bool detached = opts.detached_data;
    const std::string header_ext = std::filesystem::path(nrrd_or_nhdr_path).extension().string();
    const bool writing_nhdr = (header_ext == ".nhdr" || header_ext == ".NHDR");

    // Determine raw payload path/name
    std::string raw_name = opts.detached_data_name;
    if (raw_name.empty()) {
        raw_name = std::filesystem::path(nrrd_or_nhdr_path).stem().string() + ".raw";
    }
    const std::string raw_path =
        (dirname(nrrd_or_nhdr_path).empty() ? raw_name : (dirname(nrrd_or_nhdr_path) + "/" + raw_name));

    // Write header
    std::ofstream h(nrrd_or_nhdr_path, std::ios::binary | std::ios::trunc);
    if (!h) {
        setError(out_error, "exportNrrd: cannot open header for writing");
        return false;
    }
    h << "NRRD0005\n";
    h << "type: " << type << "\n";
    h << "dimension: 3\n";
    h << "sizes: " << vol.dims[0] << " " << vol.dims[1] << " " << vol.dims[2] << "\n";
    h << "encoding: raw\n";
    h << "endian: little\n";
    h << "spacings: " << vol.spacing[0] << " " << vol.spacing[1] << " " << vol.spacing[2] << "\n";
    h << "space origin: (" << vol.origin[0] << "," << vol.origin[1] << "," << vol.origin[2] << ")\n";
    // direction cosines (optional)
    h << "space directions: (" << vol.direction[0] * vol.spacing[0] << "," << vol.direction[1] * vol.spacing[0] << ","
      << vol.direction[2] * vol.spacing[0] << ") "
      << "(" << vol.direction[3] * vol.spacing[1] << "," << vol.direction[4] * vol.spacing[1] << ","
      << vol.direction[5] * vol.spacing[1] << ") "
      << "(" << vol.direction[6] * vol.spacing[2] << "," << vol.direction[7] * vol.spacing[2] << ","
      << vol.direction[8] * vol.spacing[2] << ")\n";

    if (detached || writing_nhdr) {
        h << "data file: " << raw_name << "\n";
    }

    h << "\n";  // blank line terminator
    if (!h) {
        setError(out_error, "exportNrrd: failed while writing header");
        return false;
    }

    const size_t bytes = vol.byteCount();
    if (detached || writing_nhdr) {
        auto st = vne::io::binaryio::writeFile(raw_path, vol.data.data(), bytes);
        if (!st) {
            setError(out_error, "exportNrrd: " + st.message);
            return false;
        }
        return true;
    }

    // Attached data: append payload right after header terminator
    h.write(reinterpret_cast<const char*>(vol.data.data()), static_cast<std::streamsize>(bytes));
    if (!h) {
        setError(out_error, "exportNrrd: failed while writing payload");
        return false;
    }
    return true;
}

}  // namespace vne::image
