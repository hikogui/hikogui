// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::Draw {

struct DrawGlobals;
inline DrawGlobals *Draw_globals = nullptr;

struct DrawGlobals {
public:
    DrawGlobals();
    ~DrawGlobals();
    DrawGlobals(DrawGlobals const &) = delete;
    DrawGlobals &operator=(DrawGlobals const &) = delete;
    DrawGlobals(DrawGlobals &&) = delete;
    DrawGlobals &operator=(DrawGlobals &&) = delete;
};

}