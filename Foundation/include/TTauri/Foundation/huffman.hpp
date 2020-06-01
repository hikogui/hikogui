// Copyright 2020 Pokitec
// All rights reserved.

#pragma once


namespace TTauri {



struct huffman_symbol {
    int symbol;
    int code;
    int length;
};

void huffman_symbol_table_from_length(std::vector<huffman_symbol> &table)
{
    // Sort the table based on the length of the code, followed by symbol
    std::sort(table.begin(), table.end(), [](let &a, let &b) {
        if (a.length == b.length) {
            return a.symbol < b.symbol;
        } else {
            return a.length < b.length;
        }
    });

    int code = 0;
    int length = 0;
    for (auto &&entry: table) {
        auto shift = entry.length - length;
        code <<= shift;

        entry.code = code;

        if (ttauri_likely(entry.length != 0)) {
            ++code;
        }
    }
}




}
