// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "data/UnicodeData.bin.inl"

namespace TTauri::Text {

TextGlobals::TextGlobals()
{
    ttauri_assert(Foundation_globals != nullptr);
    ttauri_assert(Text_globals == nullptr);
    Text_globals = this;

    Foundation_globals->addStaticResource(UnicodeData_bin_filename, UnicodeData_bin_bytes);
    unicode_data = parseResource<UnicodeData>(URL("resource:UnicodeData.bin"));

    font_book = std::make_unique<FontBook>(std::vector<URL>{
        URL::urlFromSystemFontDirectory()
    });
}

TextGlobals::~TextGlobals()
{
    ttauri_assert(Text_globals == this);
    Text_globals = nullptr;
}


}