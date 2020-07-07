// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/text/UnicodeData.hpp"
#include "ttauri/text/FontBook.hpp"
//#include "ttauri/text/Catalogue.hpp"
#include "ttauri/foundation/required.hpp"
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
