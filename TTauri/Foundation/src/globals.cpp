// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri {

FoundationGlobals::FoundationGlobals()
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals != nullptr);
    required_assert(Diagnostic_globals != nullptr);
    required_assert(Foundation_globals == nullptr);
    Foundation_globals = this;
}

FoundationGlobals::~FoundationGlobals()
{
    required_assert(Foundation_globals == this);
    Foundation_globals = nullptr;
}

void FoundationGlobals::addStaticResource(std::string const &key, gsl::span<std::byte const> value) noexcept
{
    staticResources.try_emplace(key, value);
}

gsl::span<std::byte const> FoundationGlobals::getStaticResource(std::string const &key) const
{
    let i = staticResources.find(key);
    if (i == staticResources.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<"key"_tag>(key)
        );
    }
    return i->second;
}

}