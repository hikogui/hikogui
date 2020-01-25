// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/UnicodeData.hpp"
#include "TTauri/Text/FontBook.hpp"
#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <string>
#include <mutex>

namespace TTauri::Text {

struct TextGlobals;
inline TextGlobals *Text_globals = nullptr;

struct TextGlobals {
private:
    mutable std::recursive_mutex mutex;

public:
    std::unique_ptr<UnicodeData> unicode_data;
    std::unique_ptr<FontBook> font_book;


    TextGlobals();
    ~TextGlobals();
    TextGlobals(TextGlobals const &) = delete;
    TextGlobals &operator=(TextGlobals const &) = delete;
    TextGlobals(TextGlobals &&) = delete;
    TextGlobals &operator=(TextGlobals &&) = delete;
};

}