#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 *
 * Strong, product-ready error model for vneio.
 * Stable error codes + message + optional path/subsystem.
 * Dependency-free; usable across all platforms.
 * ----------------------------------------------------------------------
 */

#include <string>
#include <utility>

namespace VNE {
namespace IO {

enum class ErrorCode_TP : int {
    OK = 0,
    UNKNOWN,
    INVALID_ARGUMENT,
    NOT_IMPLEMENTED,
    OUT_OF_MEMORY,
    FILE_NOT_FOUND,
    FILE_OPEN_FAILED,
    FILE_READ_FAILED,
    FILE_WRITE_FAILED,
    PATH_INVALID,
    UNSUPPORTED_FORMAT,
    UNSUPPORTED_FEATURE,
    PARSE_ERROR,
    DATA_CORRUPT,
    DATA_TRUNCATED,
    INVALID_DIMENSIONS,
    INVALID_PIXEL_TYPE,
    THIRD_PARTY_ERROR,
};

struct Status_C {
    ErrorCode_TP code = ErrorCode_TP::OK;
    std::string message;
    std::string path;
    std::string subsystem;

    constexpr bool Ok() const noexcept { return code == ErrorCode_TP::OK; }
    explicit constexpr operator bool() const noexcept { return Ok(); }

    static Status_C OkStatus() { return {}; }

    static Status_C Make(ErrorCode_TP c, std::string msg, std::string p = {}, std::string sub = {}) {
        Status_C s;
        s.code = c;
        s.message = std::move(msg);
        s.path = std::move(p);
        s.subsystem = std::move(sub);
        return s;
    }
};

template<typename T>
struct Result_T {
    T value{};
    Status_C status{};
    constexpr bool Ok() const noexcept { return status.Ok(); }
    explicit constexpr operator bool() const noexcept { return Ok(); }
};

}  // namespace IO
}  // namespace VNE
