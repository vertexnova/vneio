/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * MetaImage exporter (MHD/MHA).
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume_exporter.h"
#include "vertexnova/io/common/binary_io.h"

#include <fstream>
#include <filesystem>

namespace vne::image {

namespace {

void setError(std::string* out_error, const std::string& msg) {
    if (out_error) {
        *out_error = msg;
    }
}

std::string pixelTypeToMet(VolumePixelType t) {
    switch (t) {
        case VolumePixelType::eUint8:
            return "MET_UCHAR";
        case VolumePixelType::eInt8:
            return "MET_CHAR";
        case VolumePixelType::eUint16:
            return "MET_USHORT";
        case VolumePixelType::eInt16:
            return "MET_SHORT";
        case VolumePixelType::eUint32:
            return "MET_UINT";
        case VolumePixelType::eInt32:
            return "MET_INT";
        case VolumePixelType::eFloat32:
            return "MET_FLOAT";
        case VolumePixelType::eFloat64:
            return "MET_DOUBLE";
        case VolumePixelType::eUnknown:
            break;
    }
    return "MET_UNKNOWN";
}

std::string dirname(const std::string& p) {
    std::filesystem::path pp(p);
    return pp.parent_path().string();
}

}  // namespace

bool exportMhd(const std::string& mhd_or_mha_path,
               const Volume& vol,
               const MhdExportOptions& opts,
               std::string* out_error) {
    if (vol.isEmpty()) {
        setError(out_error, "exportMhd: volume is empty");
        return false;
    }
    if (vol.components != 1) {
        setError(out_error, "exportMhd: only scalar volumes (components==1) are supported");
        return false;
    }
    const std::string et = pixelTypeToMet(vol.pixel_type);
    if (et == "MET_UNKNOWN") {
        setError(out_error, "exportMhd: unsupported pixel type");
        return false;
    }

    const std::string ext = std::filesystem::path(mhd_or_mha_path).extension().string();
    const bool writing_mha = (ext == ".mha" || ext == ".MHA") || opts.inline_data;

    std::string raw_name = opts.raw_data_name;
    if (raw_name.empty()) {
        raw_name = std::filesystem::path(mhd_or_mha_path).stem().string() + ".raw";
    }
    const std::string raw_path =
        (dirname(mhd_or_mha_path).empty() ? raw_name : (dirname(mhd_or_mha_path) + "/" + raw_name));

    std::ofstream h(mhd_or_mha_path, std::ios::binary | std::ios::trunc);
    if (!h) {
        setError(out_error, "exportMhd: cannot open header for writing");
        return false;
    }

    h << "ObjectType = Image\n";
    h << "NDims = 3\n";
    h << "DimSize = " << vol.dims[0] << " " << vol.dims[1] << " " << vol.dims[2] << "\n";
    h << "ElementType = " << et << "\n";
    h << "ElementSpacing = " << vol.spacing[0] << " " << vol.spacing[1] << " " << vol.spacing[2] << "\n";
    h << "Position = " << vol.origin[0] << " " << vol.origin[1] << " " << vol.origin[2] << "\n";
    h << "ElementByteOrderMSB = False\n";

    if (writing_mha) {
        h << "ElementDataFile = LOCAL\n\n";
        const size_t bytes = vol.byteCount();
        h.write(reinterpret_cast<const char*>(vol.data.data()), static_cast<std::streamsize>(bytes));
        if (!h) {
            setError(out_error, "exportMhd: failed while writing inline payload");
            return false;
        }
        return true;
    }

    h << "ElementDataFile = " << raw_name << "\n\n";
    if (!h) {
        setError(out_error, "exportMhd: failed while writing header");
        return false;
    }

    const size_t bytes = vol.byteCount();
    auto st = vne::io::binaryio::writeFile(raw_path, vol.data.data(), bytes);
    if (!st) {
        setError(out_error, "exportMhd: " + st.message);
        return false;
    }
    return true;
}

}  // namespace vne::image
