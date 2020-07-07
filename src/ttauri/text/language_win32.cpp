// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/logger.hpp"
#include "language.hpp"
#include <Windows.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.UserProfile.h>

namespace tt {

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::UserProfile;

std::vector<std::string> language::get_preferred_language_tags() noexcept
{
    winrt::init_apartment();

    std::vector<std::string> r;
    for (const auto& lang : GlobalizationPreferences::Languages()) {
        r.push_back(tt::to_string(lang));
    }
    return r;
}

}