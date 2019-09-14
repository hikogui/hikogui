// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::GUI::Widgets {

struct WidgetsGlobals;
inline WidgetsGlobals *Widgets_globals = nullptr;

struct WidgetsGlobals {
public:
    WidgetsGlobals();
    ~WidgetsGlobals();
    WidgetsGlobals(WidgetsGlobals const &) = delete;
    WidgetsGlobals &operator=(WidgetsGlobals const &) = delete;
    WidgetsGlobals(WidgetsGlobals &&) = delete;
    WidgetsGlobals &operator=(WidgetsGlobals &&) = delete;
};

}