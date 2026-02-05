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

#include "vertexnova/io/dicom/dicom_loader.h"

#include <memory>

namespace vne::dicom {

class DicomLoaderRegistry {
   public:
    [[nodiscard]] static std::unique_ptr<IDicomLoader> create();
};

}  // namespace vne::dicom
