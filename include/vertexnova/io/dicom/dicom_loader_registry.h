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

/**
 * @file dicom_loader_registry.h
 * @brief Factory for DICOM loader (GDCM/DCMTK at link time; stub if none).
 */

/**
 * @class DicomLoaderRegistry
 * @brief Factory for DICOM loader implementation.
 *
 * create() returns the configured loader or a stub that fails with a clear message
 * when no backend (VNEIO_WITH_GDCM / VNEIO_WITH_DCMTK) is built.
 */
class DicomLoaderRegistry {
   public:
    /**
     * @brief Create a DICOM loader instance (caller owns).
     * @return Loader or stub; never nullptr.
     */
    [[nodiscard]] static std::unique_ptr<IDicomLoader> create();
};

}  // namespace vne::dicom
