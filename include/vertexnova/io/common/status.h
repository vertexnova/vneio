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

/**
 * @file status.h
 * @brief Error model for vneio: stable error codes, message, optional path/subsystem.
 */

/**
 * @enum ErrorCode
 * @brief Stable error codes for load and export operations.
 */
enum class ErrorCode : int {
    eOk = 0,            //!< Success.
    eUnknown,           //!< Unknown error.
    eInvalidArgument,   //!< Invalid parameter.
    eNotImplemented,    //!< Feature not implemented.
    eOutOfMemory,       //!< Allocation failed.
    eFileNotFound,      //!< File does not exist.
    eFileOpenFailed,    //!< Failed to open file.
    eFileReadFailed,    //!< Read error.
    eFileWriteFailed,   //!< Write error.
    ePathInvalid,       //!< Invalid path.
    eUnsupportedFormat, //!< Format not supported.
    eUnsupportedFeature,//!< Feature not supported.
    eParseError,        //!< Parse failed.
    eDataCorrupt,       //!< Corrupt data.
    eDataTruncated,     //!< Truncated or incomplete data.
    eInvalidDimensions, //!< Invalid dimensions.
    eInvalidPixelType,  //!< Unsupported pixel type.
    eThirdPartyError,   //!< Error from third-party library.
};

/**
 * @struct Status
 * @brief Result of an operation: error code plus optional message, path, subsystem.
 */
struct Status {
    ErrorCode code = ErrorCode::eOk;  //!< Error code.
    std::string message;              //!< Human-readable message.
    std::string path;                 //!< Optional file or resource path.
    std::string subsystem;            //!< Optional subsystem name (e.g. "BinaryIO", "NrrdLoader").

    /** @brief Returns true if code is eOk. */
    [[nodiscard]] constexpr bool ok() const noexcept { return code == ErrorCode::eOk; }
    explicit constexpr operator bool() const noexcept { return ok(); }

    /** @brief Returns a success status. */
    static Status okStatus() { return {}; }

    /**
     * @brief Build a status with the given code, message, and optional path/subsystem.
     * @param c Error code.
     * @param msg Message string.
     * @param p Optional path.
     * @param sub Optional subsystem name.
     * @return Constructed Status.
     */
    static Status make(ErrorCode c, std::string msg, std::string p = {}, std::string sub = {}) {
        Status s;
        s.code = c;
        s.message = std::move(msg);
        s.path = std::move(p);
        s.subsystem = std::move(sub);
        return s;
    }
};

/**
 * @struct Result
 * @brief Typed result: value plus status. Use ok() to check success; value is valid when ok().
 */
template<typename T>
struct Result {
    T value{};       //!< The loaded or computed value.
    Status status{}; //!< Status of the operation.
    /** @brief Returns true if the operation succeeded. */
    [[nodiscard]] constexpr bool ok() const noexcept { return status.ok(); }
    explicit constexpr operator bool() const noexcept { return ok(); }
};

}  // namespace io
}  // namespace vne
