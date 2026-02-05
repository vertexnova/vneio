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

#include <string>
#include <utility>

namespace vne {
namespace io {

enum class ErrorCode : int {
    eOk = 0,
    eUnknown,
    eInvalidArgument,
    eNotImplemented,
    eOutOfMemory,
    eFileNotFound,
    eFileOpenFailed,
    eFileReadFailed,
    eFileWriteFailed,
    ePathInvalid,
    eUnsupportedFormat,
    eUnsupportedFeature,
    eParseError,
    eDataCorrupt,
    eDataTruncated,
    eInvalidDimensions,
    eInvalidPixelType,
    eThirdPartyError,
};

struct Status {
    ErrorCode code = ErrorCode::eOk;
    std::string message;
    std::string path;
    std::string subsystem;

    [[nodiscard]] constexpr bool ok() const noexcept { return code == ErrorCode::eOk; }
    explicit constexpr operator bool() const noexcept { return ok(); }

    static Status okStatus() { return {}; }

    static Status make(ErrorCode c, std::string msg, std::string p = {}, std::string sub = {}) {
        Status s;
        s.code = c;
        s.message = std::move(msg);
        s.path = std::move(p);
        s.subsystem = std::move(sub);
        return s;
    }
};

template<typename T>
struct Result {
    T value{};
    Status status{};
    [[nodiscard]] constexpr bool ok() const noexcept { return status.ok(); }
    explicit constexpr operator bool() const noexcept { return ok(); }
};

}  // namespace io
}  // namespace vne
