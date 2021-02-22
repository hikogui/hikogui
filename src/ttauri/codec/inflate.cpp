// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "inflate.hpp"
#include "../bits.hpp"
#include "../placement.hpp"
#include "../huffman.hpp"
#include <array>

namespace tt {

static void inflate_copy_block(std::span<std::byte const> bytes, ssize_t &bit_offset, ssize_t max_size, bstring &r)
{
    auto offset = (bit_offset + 7) / 8;

    auto LEN = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
    [[maybe_unused]] auto NLEN = make_placement_ptr<little_uint16_buf_t>(bytes, offset);

    tt_parse_check((offset + LEN->value()) <= std::ssize(bytes), "input buffer overrun");
    tt_parse_check((std::ssize(r) + LEN->value()) <= max_size, "output buffer overrun");
    r.append(&bytes[offset], LEN->value());

    bit_offset = offset * 8;
}

[[nodiscard]] static int inflate_decode_length(
    std::span<std::byte const> bytes,
    ssize_t &bit_offset,
    int symbol)
{
    switch (symbol) {
    case 257: return 3;
    case 258: return 4;
    case 259: return 5;
    case 260: return 6;
    case 261: return 7;
    case 262: return 8;
    case 263: return 9;
    case 264: return 10;
    case 265: return get_bits(bytes, bit_offset, 1) + 11;
    case 266: return get_bits(bytes, bit_offset, 1) + 13;
    case 267: return get_bits(bytes, bit_offset, 1) + 15;
    case 268: return get_bits(bytes, bit_offset, 1) + 17;
    case 269: return get_bits(bytes, bit_offset, 2) + 19;
    case 270: return get_bits(bytes, bit_offset, 2) + 23;
    case 271: return get_bits(bytes, bit_offset, 2) + 27;
    case 272: return get_bits(bytes, bit_offset, 2) + 31;
    case 273: return get_bits(bytes, bit_offset, 3) + 35;
    case 274: return get_bits(bytes, bit_offset, 3) + 43;
    case 275: return get_bits(bytes, bit_offset, 3) + 51;
    case 276: return get_bits(bytes, bit_offset, 3) + 59;
    case 277: return get_bits(bytes, bit_offset, 4) + 67;
    case 278: return get_bits(bytes, bit_offset, 4) + 83;
    case 279: return get_bits(bytes, bit_offset, 4) + 99;
    case 280: return get_bits(bytes, bit_offset, 4) + 115;
    case 281: return get_bits(bytes, bit_offset, 5) + 131;
    case 282: return get_bits(bytes, bit_offset, 5) + 163;
    case 283: return get_bits(bytes, bit_offset, 5) + 195;
    case 284: return get_bits(bytes, bit_offset, 5) + 227;
    case 285: return 258;
    default:
        throw parse_error("Literal/Length symbol out of range {}", symbol); 
    }
}

[[nodiscard]] static int inflate_decode_distance(
    std::span<std::byte const> bytes,
    ssize_t &bit_offset,
    int symbol)
{
    switch (symbol) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 3;
    case 3: return 4;
    case 4: return get_bits(bytes, bit_offset, 1) + 5;
    case 5: return get_bits(bytes, bit_offset, 1) + 7;
    case 6: return get_bits(bytes, bit_offset, 2) + 9;
    case 7: return get_bits(bytes, bit_offset, 2) + 13;
    case 8: return get_bits(bytes, bit_offset, 3) + 17;
    case 9: return get_bits(bytes, bit_offset, 3) + 25;
    case 10: return get_bits(bytes, bit_offset, 4) + 33;
    case 11: return get_bits(bytes, bit_offset, 4) + 49;
    case 12: return get_bits(bytes, bit_offset, 5) + 65;
    case 13: return get_bits(bytes, bit_offset, 5) + 97;
    case 14: return get_bits(bytes, bit_offset, 6) + 129;
    case 15: return get_bits(bytes, bit_offset, 6) + 193;
    case 16: return get_bits(bytes, bit_offset, 7) + 257;
    case 17: return get_bits(bytes, bit_offset, 7) + 385;
    case 18: return get_bits(bytes, bit_offset, 8) + 513;
    case 19: return get_bits(bytes, bit_offset, 8) + 769;
    case 20: return get_bits(bytes, bit_offset, 9) + 1025;
    case 21: return get_bits(bytes, bit_offset, 9) + 1537;
    case 22: return get_bits(bytes, bit_offset, 10) + 2049;
    case 23: return get_bits(bytes, bit_offset, 10) + 3073;
    case 24: return get_bits(bytes, bit_offset, 11) + 4097;
    case 25: return get_bits(bytes, bit_offset, 11) + 6145;
    case 26: return get_bits(bytes, bit_offset, 12) + 8193;
    case 27: return get_bits(bytes, bit_offset, 12) + 12289;
    case 28: return get_bits(bytes, bit_offset, 13) + 16385;
    case 29: return get_bits(bytes, bit_offset, 13) + 24577;
    default:
        throw parse_error("Distance symbol out of range {}", symbol); 
    }
}

static void inflate_block(
    std::span<std::byte const> bytes,
    ssize_t &bit_offset,
    ssize_t max_size,
    huffman_tree<int16_t> const &literal_tree,
    huffman_tree<int16_t> const &distance_tree,
    bstring &r)
{
    while (true) {
        // Test only every get_symbol, the trailer is at least 32 bits (Checksum)
        // - 15 bits maximum huffman code.
        // -  5 bits extra length.
        // -  7 bits rounding up to byte.
        tt_parse_check(((bit_offset + 27) >> 3) <= std::ssize(bytes), "Input buffer overrun");

        auto literal_symbol = literal_tree.get_symbol(bytes, bit_offset);

        if (literal_symbol <= 255) {
            tt_parse_check(std::ssize(r) < max_size, "Output buffer overrun");
            r.push_back(static_cast<std::byte>(literal_symbol));

        } else if (literal_symbol == 256) {
            // End-of-block.
            return;

        } else {
            auto length = inflate_decode_length(bytes, bit_offset, literal_symbol); 
            tt_parse_check(std::ssize(r) + length <= max_size, "Output buffer overrun");

            // Test only every get_symbol, the trailer is at least 32 bits (Checksum)
            // - 15 bits maximum huffman code.
            // -  7 bits rounding up to byte.
            tt_parse_check(((bit_offset + 22) >> 3) <= std::ssize(bytes), "Input buffer overrun");
            auto distance_symbol = distance_tree.get_symbol(bytes, bit_offset);

            // Test only every inflate_decode_distance, the trailer is at least 32 bits (Checksum)
            // - 13 bits extra length.
            // -  7 bits rounding up to byte.
            tt_parse_check(((bit_offset + 20) >> 3) <= std::ssize(bytes), "Input buffer overrun");
            auto distance = inflate_decode_distance(bytes, bit_offset, distance_symbol);

            tt_parse_check(distance <= std::ssize(r), "Distance beyond start of decompressed data");
            auto src_i = std::ssize(r) - distance;
            for (auto i = 0; i != length; ++i) {
                r.push_back(r[src_i++]);
            }
        }
    }
}

huffman_tree<int16_t> deflate_fixed_literal_tree = []() {
    std::vector<int> lengths;

    for (int i = 0; i <= 143; ++i) {
        lengths.push_back(8);
    }
    for (int i = 144; i <= 255; ++i) {
        lengths.push_back(9);
    }
    for (int i = 256; i <= 279; ++i) {
        lengths.push_back(7);
    }
    for (int i = 280; i <= 287; ++i) {
        lengths.push_back(8);
    }

    return huffman_tree<int16_t>::from_lengths(lengths);
}();

huffman_tree<int16_t> deflate_fixed_distance_tree = []() {
    std::vector<int> lengths;

    for (int i = 0; i <= 31; ++i) {
        lengths.push_back(5);
    }

    return huffman_tree<int16_t>::from_lengths(lengths);
}();



static void inflate_fixed_block(std::span<std::byte const> bytes, ssize_t &bit_offset, ssize_t max_size, bstring &r)
{
    inflate_block(bytes, bit_offset, max_size, deflate_fixed_literal_tree, deflate_fixed_distance_tree, r);
}

[[nodiscard]] static huffman_tree<int16_t> inflate_code_lengths(std::span<std::byte const> bytes, ssize_t &bit_offset, int nr_symbols)
{
    // The symbols are in different order in the table.
    constexpr auto symbols = std::array{
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    tt_parse_check(((bit_offset + (3 * static_cast<ssize_t>(nr_symbols)) + 7) >> 3) <= std::ssize(bytes), "Input buffer overrun");

    auto lengths = std::vector<int>(std::ssize(symbols), 0);
    for (int i = 0; i != nr_symbols; ++i) {
        ttlet symbol = symbols[i];
        lengths[symbol] = get_bits(bytes, bit_offset, 3);
    }
    return huffman_tree<int16_t>::from_lengths(std::move(lengths));
}

std::vector<int> inflate_lengths(
    std::span<std::byte const> bytes,
    ssize_t &bit_offset,
    int nr_symbols,
    huffman_tree<int16_t> const &code_length_tree)
{
    auto r = std::vector<int>{};
    r.reserve(nr_symbols);

    auto prev_length = 0;
    while (std::ssize(r) < nr_symbols) {
        // Test only every get_symbol, the trailer is at least 32 bits (Checksum)
        // -  7 bits maximum huffman code.
        // -  7 bits extra length.
        // -  7 bits rounding up to byte.
        tt_parse_check(((bit_offset + 21) >> 3) <= std::ssize(bytes), "Input buffer overrun");
        auto symbol = code_length_tree.get_symbol(bytes, bit_offset);

        switch (symbol) {
        case 16: {
                auto copy_length = get_bits(bytes, bit_offset, 2) + 3;
                while (copy_length--) {
                    r.push_back(prev_length);
                }
            } break;
        case 17: {
                auto copy_length = get_bits(bytes, bit_offset, 3) + 3;
                while (copy_length--) {
                    r.push_back(0);
                }
            } break;
        case 18: {
                auto copy_length = get_bits(bytes, bit_offset, 7) + 11;
                while (copy_length--) {
                    r.push_back(0);
                }
            } break;
        default:
            r.push_back(prev_length = symbol);
        }
    }

    return r;
}

void inflate_dynamic_block(std::span<std::byte const> bytes, ssize_t &bit_offset, ssize_t max_size, bstring &r)
{
    // Test all lengths, the trailer is at least 32 bits (Checksum)
    // - 14 bits lengths
    // -  7 bits rounding up to byte.
    tt_parse_check(((bit_offset + 21) >> 3) <= std::ssize(bytes), "Input buffer overrun");
    ttlet HLIT = get_bits(bytes, bit_offset, 5);
    ttlet HDIST = get_bits(bytes, bit_offset, 5);
    ttlet HCLEN = get_bits(bytes, bit_offset, 4);

    ttlet code_length_tree = inflate_code_lengths(bytes, bit_offset, HCLEN + 4);

    ttlet lengths = inflate_lengths(bytes, bit_offset, HLIT + HDIST + 258, code_length_tree);
    tt_parse_check(lengths[256] != 0, "The end-of-block symbol must be in the table");

    ttlet lengths_ptr = lengths.data();
    ttlet literal_tree = huffman_tree<int16_t>::from_lengths(lengths_ptr, HLIT + 257);
    ttlet distance_tree = huffman_tree<int16_t>::from_lengths(&lengths_ptr[HLIT + 257], HDIST + 1);

    inflate_block(bytes, bit_offset, max_size, literal_tree, distance_tree, r);
}

bstring inflate(std::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size)
{
    ssize_t bit_offset = offset * 8;

    auto r = bstring{};

    bool BFINAL;
    do {
        // Test all lengths, the trailer is at least 32 bits (Checksum)
        // - 3 bits header
        // - 7 bits rounding up to byte.
        tt_parse_check(((bit_offset + 10) >> 3) <= std::ssize(bytes), "Input buffer overrun");

        BFINAL = get_bits(bytes, bit_offset, 1);
        ttlet BTYPE = get_bits(bytes, bit_offset, 2);

        switch (BTYPE) {
        case 0:
            inflate_copy_block(bytes, bit_offset, max_size - std::ssize(r), r);
            break;
        case 1:
            inflate_fixed_block(bytes, bit_offset, max_size - std::ssize(r), r);
            break;
        case 2:
            inflate_dynamic_block(bytes, bit_offset, max_size - std::ssize(r), r);
            break;
        default:
            throw parse_error("Reserved block type");
        }

    } while (!BFINAL);

    offset = (bit_offset + 7) / 8;
    return r;
}

}