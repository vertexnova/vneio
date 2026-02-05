#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/image/volume.h"

#include <string>
#include <unordered_map>

namespace vne::dicom {

/**
 * @file dicom_series.h
 * @brief DICOM series container: reconstructed volume plus common metadata.
 */

/**
 * @struct DicomSeries
 * @brief Lightweight DICOM series result: volume plus common tags as strings.
 */
struct DicomSeries {
    vne::image::Volume volume;                        //!< Reconstructed 3D volume.
    std::unordered_map<std::string, std::string> meta; //!< Additional tags (e.g. for UI).
    std::string series_uid;   //!< Series Instance UID.
    std::string study_uid;    //!< Study Instance UID.
    std::string patient_id;   //!< Patient ID.
    std::string modality;    //!< Modality (e.g. CT, MR).
};

}  // namespace vne::dicom
