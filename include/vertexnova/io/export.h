#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * @file export.h
 * @brief `VNEIO_API` — DLL export/import macro for shared-library builds.
 *
 * On Windows, define @c VNEIO_BUILDING_DLL when compiling the library and
 * @c VNEIO_DLL when linking the DLL from an app.
 * On Unix shared builds, @c VNEIO_BUILDING_DLL selects default visibility; otherwise empty.
 */

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(VNEIO_BUILDING_DLL)
#define VNEIO_API __declspec(dllexport)
#elif defined(VNEIO_DLL)
#define VNEIO_API __declspec(dllimport)
#else
#define VNEIO_API
#endif
#else
#if defined(VNEIO_BUILDING_DLL) && (defined(__GNUC__) || defined(__clang__))
#define VNEIO_API __attribute__((visibility("default")))
#else
#define VNEIO_API
#endif
#endif
