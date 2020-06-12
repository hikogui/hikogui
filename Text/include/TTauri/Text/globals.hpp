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

namespace tt {

inline std::unique_ptr<UnicodeData> unicodeData;


/** Startup the Text library.
*/
void text_startup();

/** Shutdown the Text library.
*/
void text_shutdown();

}
