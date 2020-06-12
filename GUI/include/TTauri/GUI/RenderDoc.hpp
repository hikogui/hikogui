// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

namespace tt {

class RenderDoc {
    /** Pointer to the RenderDoc API struct.
     */
    void *api = nullptr;

public:
    RenderDoc() noexcept;

    void setOverlay(bool frameRate, bool frameNumber, bool captureList) noexcept;

};

inline RenderDoc *renderDoc;

}