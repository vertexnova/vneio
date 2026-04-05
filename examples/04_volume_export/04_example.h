#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 04: Volume export round-trip — synthesise a volume and prove
 * it survives all four export / reload paths:
 *   .nrrd (attached)  |  .nhdr + .raw (detached NRRD)
 *   .mhd  + .raw      |  .mha (inline MHD)
 * ----------------------------------------------------------------------
 */

namespace vne::io::examples {

[[nodiscard]] int runVolumeExportExample();

}  // namespace vne::io::examples
