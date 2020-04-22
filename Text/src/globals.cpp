// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "data/UnicodeData.bin.inl"

namespace TTauri::Text {

void startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    TTauri::startup();
    LOG_INFO("TTauri::Text startup");

    addStaticResource(UnicodeData_bin_filename, UnicodeData_bin_bytes);

    unicodeData = parseResource<UnicodeData>(URL("resource:UnicodeData.bin"));

    fontBook = new FontBook(std::vector<URL>{
        URL::urlFromSystemFontDirectory()
    });
}

void shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("TTauri::Text shutdown");

    unicodeData.release();
    delete fontBook;

    TTauri::shutdown();
}

}