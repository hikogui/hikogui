// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "language.hpp"
#include <Windows.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.UserProfile.h>

namespace tt {

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::UserProfile;

std::vector<std::string> readOSPreferedLanguages() noexcept
{
    winrt::init_apartment();

    std::vector<std::string> r;
    for (const auto& lang : GlobalizationPreferences::Languages()) {
        r.push_back(tt::to_string(lang));
    }
    return r;
}

}