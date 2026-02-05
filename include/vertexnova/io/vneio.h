#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   January 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * @file vneio.h
 * @brief Umbrella header for VneIo: mesh, image, volume, DICOM, and asset IO.
 */

// Common (Status, BinaryIO)
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/common/binary_io.h"

// Asset io (LoadRequest, registry, loader interfaces)
#include "vertexnova/io/load_request.h"
#include "vertexnova/io/asset_loader.h"
#include "vertexnova/io/asset_io.h"

// Mesh (requires Assimp when building)
#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/mesh/mesh_loader_registry.h"
#include "vertexnova/io/mesh/mesh_exporter.h"

// Image (requires stb when building)
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/image_loader.h"
#include "vertexnova/io/image/volume_loader.h"
#include "vertexnova/io/image/stb_image_loader.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
#include "vertexnova/io/image/volume_exporter.h"

// DICOM (optional backends: GDCM/DCMTK)
#include "vertexnova/io/dicom/dicom_series.h"
#include "vertexnova/io/dicom/dicom_loader.h"
#include "vertexnova/io/dicom/dicom_loader_registry.h"
