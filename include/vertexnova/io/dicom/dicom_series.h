#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume.h"

#include <string>
#include <unordered_map>

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
