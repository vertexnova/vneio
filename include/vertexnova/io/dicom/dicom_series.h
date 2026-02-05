#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * DICOM series container.
 *
 * This is intentionally lightweight:
 *  - `volume` contains the reconstructed 3D pixels
 *  - `meta` exposes common tags as strings for UI and debugging
 *
 * Full DICOM fidelity (private tags, multi-frame, RTSTRUCT, etc.) should
 * be handled by a dedicated library (GDCM / DCMTK) and mapped into this
 * structure at the app boundary.
 * ----------------------------------------------------------------------
 */

#include <string>
#include <unordered_map>

#include "vertexnova/io/image/volume.h"

namespace vne::dicom {

struct DicomSeries {
    vne::image::Volume volume;
    std::unordered_map<std::string, std::string> meta;
    std::string series_uid;
    std::string study_uid;
    std::string patient_id;
    std::string modality;
};

}  // namespace vne::dicom
