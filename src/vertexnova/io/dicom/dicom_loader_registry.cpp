/* ---------------------------------------------------------------------
 * Copyright (c) 2025 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 * ----------------------------------------------------------------------
 */

#include "vertexnova/io/dicom/dicom_loader_registry.h"
#include "vertexnova/io/common/status.h"
#include "vertexnova/io/load_request.h"

namespace vne::dicom {

namespace {
class NullDicomLoader final : public IDicomLoader {
   public:
    vne::io::LoadResult<DicomSeries_C> loadDicomSeries(const vne::io::LoadRequest& request) override {
        vne::io::LoadResult<DicomSeries_C> result;
        if (!loadDirectory(request.uri, result.value)) {
            result.status = vne::io::Status::make(vne::io::ErrorCode::eNotImplemented,
                                                   getLastError(),
                                                   request.uri,
                                                   "DicomLoader");
            return result;
        }
        result.status = vne::io::Status::okStatus();
        return result;
    }

    bool loadDirectory(const std::string& directory_path, DicomSeries_C& out_series) override {
        (void)directory_path;
        out_series = {};
        last_error_ = "DICOM support not built. Define VNEIO_WITH_GDCM or VNEIO_WITH_DCMTK and link the library.";
        return false;
    }

    std::string getLastError() const override { return last_error_; }

   private:
    std::string last_error_;
};
}  // namespace

std::unique_ptr<IDicomLoader> DicomLoaderRegistry::Create() {
#if defined(VNEIO_WITH_GDCM)
    // TODO: Add GDCM loader implementation (recommended OSS backend).
    return std::make_unique<NullDicomLoader>();
#elif defined(VNEIO_WITH_DCMTK)
    // TODO: Add DCMTK loader implementation.
    return std::make_unique<NullDicomLoader>();
#else
    return std::make_unique<NullDicomLoader>();
#endif
}

}  // namespace vne::dicom
