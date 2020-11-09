// Copyright 2020 Pokitec
// All rights reserved.

#include "language.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include <Windows.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.UserProfile.h>

namespace tt {

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::UserProfile;

std::vector<language_tag> language::read_os_preferred_languages() noexcept
{
    winrt::init_apartment();

    std::vector<language_tag> r;
    for (const auto& lang : GlobalizationPreferences::Languages()) {
        r.emplace_back(tt::to_string(lang));
    }
    return r;
}

}