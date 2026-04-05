/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Example 07: Load-time performance benchmark — measures wall-clock
 * throughput for all three asset loaders using fully synthetic data.
 * No testdata dependency; all assets are generated and written to /tmp.
 * ----------------------------------------------------------------------
 */

#include "07_example.h"

int main() {
    return vne::io::examples::runPerformanceExample();
}
