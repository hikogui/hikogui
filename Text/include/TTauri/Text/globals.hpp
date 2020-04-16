// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/UnicodeData.hpp"
#include "TTauri/Text/FontBook.hpp"
//#include "TTauri/Text/Catalogue.hpp"
#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <string>
#include <mutex>

namespace TTauri::Text {

inline std::unique_ptr<UnicodeData> unicodeData;

inline FontBook *fontBook;

/** Reference counter to determine the amount of startup/shutdowns.
*/
inline std::atomic<uint64_t> startupCount = 0;

/** Startup the Text library.
*/
void startup();

/** Shutdown the Text library.
*/
void shutdown();

}
