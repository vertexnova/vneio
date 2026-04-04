#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 06: Unified AssetIO registry — register all loaders and load
 * all three asset types through a single interface. Demonstrates the
 * LoadResult<T> pattern and error code handling.
 * ----------------------------------------------------------------------
 */

namespace vne::io::examples {

[[nodiscard]] int runAssetRegistryExample();

}  // namespace vne::io::examples
