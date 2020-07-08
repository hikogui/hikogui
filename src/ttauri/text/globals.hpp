// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "UnicodeData.hpp"
#include "FontBook.hpp"
#include "../required.hpp"
#include <cstdint>
#include <string>
#include <mutex>

namespace tt {

inline std::unique_ptr<UnicodeData> unicodeData;


/** Startup the Text library.
*/
void text_startup();

/** Shutdown the Text library.
*/
void text_shutdown();

}
