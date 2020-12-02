// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <memory>

namespace tt {

class RenderDoc {
public:
    static inline std::unique_ptr<RenderDoc> global;

    RenderDoc() noexcept;

    void set_overlay(bool frameRate, bool frameNumber, bool captureList) noexcept;

private:
    /** Pointer to the RenderDoc API struct.
     */
    void *api = nullptr;
};

}
