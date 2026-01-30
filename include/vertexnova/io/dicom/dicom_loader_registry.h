#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * DICOM loader registry.
 *
 * This allows your engine/app to depend on vneio interfaces only,
 * while selecting an implementation at link-time.
 *
 * Build flags:
 *  - VNEIO_WITH_GDCM
 *  - VNEIO_WITH_DCMTK
 * ----------------------------------------------------------------------
 */

#include <memory>

#include "vertexnova/io/dicom/dicom_loader.h"

namespace vne::dicom {

class DicomLoaderRegistry {
   public:
    static std::unique_ptr<IDicomLoader> Create();
};

}  // namespace vne::dicom
