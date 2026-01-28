#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Umbrella header for VneIo: mesh and image from VertexNova core.
 * ----------------------------------------------------------------------
 */

// Mesh (requires Assimp when building)
#include "vertexnova/io/mesh/mesh.h"
#include "vertexnova/io/mesh/mesh_loader.h"
#include "vertexnova/io/mesh/assimp_loader.h"
#include "vertexnova/io/mesh/mesh_loader_registry.h"

// Image (requires stb when building)
#include "vertexnova/io/image/image.h"
#include "vertexnova/io/image/volume.h"
#include "vertexnova/io/image/nrrd_loader.h"
#include "vertexnova/io/image/mhd_loader.h"
