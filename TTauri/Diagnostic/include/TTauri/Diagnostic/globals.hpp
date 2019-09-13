// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include <cstdint>
#include <string>

namespace TTauri {

struct DiagnosticGlobals;
inline DiagnosticGlobals *Diagnostic_globals = nullptr;

struct DiagnosticGlobals {
private:

public:
    DiagnosticGlobals();
    ~DiagnosticGlobals();
    DiagnosticGlobals(DiagnosticGlobals const &) = delete;
    DiagnosticGlobals &operator=(DiagnosticGlobals const &) = delete;
    DiagnosticGlobals(DiagnosticGlobals &&) = delete;
    DiagnosticGlobals &operator=(DiagnosticGlobals &&) = delete;
};

}