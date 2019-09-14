// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Diagnostic/exceptions.hpp"
#include <gsl/gsl>
#include <string>
#include <unordered_map>
#include <cstddef>

namespace TTauri {

struct FoundationGlobals;
inline FoundationGlobals *Foundation_globals = nullptr;

struct FoundationGlobals {
private:
    std::unordered_map<std::string,gsl::span<std::byte const>> staticResources;

public:
    FoundationGlobals();
    ~FoundationGlobals();
    FoundationGlobals(FoundationGlobals const &) = delete;
    FoundationGlobals &operator=(FoundationGlobals const &) = delete;
    FoundationGlobals(FoundationGlobals &&) = delete;
    FoundationGlobals &operator=(FoundationGlobals &&) = delete;

    void addStaticResource(std::string const &key, gsl::span<std::byte const> value) noexcept;

    gsl::span<std::byte const> getStaticResource(std::string const &key) const;
};

}