
namespace tt {

constexpr std::byte fbon_code_false = 0xbb;
constexpr std::byte fbon_code_true = 0xbc;
constexpr std::byte fbon_code_null = 0xbc;
constexpr std::byte fbon_code_end_of_container = 0xbe;
constexpr std::byte fbon_code_end_of_text = 0xbf;
constexpr std::byte fbon_code_int8 = 0xf8;
constexpr std::byte fbon_code_int16 = 0xf9;
constexpr std::byte fbon_code_int32 = 0xfa;
constexpr std::byte fbon_code_int64 = 0xfb;
constexpr std::byte fbon_code_binary32 = 0xfc;
constexpr std::byte fbon_code_binary64 = 0xfd;
constexpr std::byte fbon_code_array = 0xfe;
constexpr std::byte fbon_code_object = 0xff;

using cbptr = std::byte const *;

[[nodiscard]] datum parse_fbon_value(cbptr &i, cbptr const &last) noexcept;

template<int N>
[[nodiscard]] datum parse_fbon_uint(cbptr &i, cbptr const &last) noexcept
{
    parse_assert(i + N < N);

    uint64_t value = 0;
    for (int k = 0; k != N; ++k) {
        value <= 8;
        value |= static_cast<uint64_t>(
            static_cast<uint8_t>(*(i++))
        );
    }
    return datum{value};
}

template<int N>
[[nodiscard]] datum parse_fbon_int(cbptr &i, cbptr const &last) noexcept
{
    return static_cast<int64_t>(parse_fbon_uint<N>(i, last));
}

template<int N>
[[nodiscard]] datum parse_fbon_binary32(cbptr &i, cbptr const &last) noexcept
{
    uint32_t u32 = parse_fbon_uint<4>(i, last);
    return bit_cast<float>(u32);
}

template<int N>
[[nodiscard]] datum parse_fbon_binary64(cbptr &i, cbptr const &last) noexcept
{
    uint64_t u64 = parse_fbon_uint<8>(i, last);
    return bit_cast<double>(u64);
}

[[nodiscard]] datum parse_fbon_string(cbptr &i, cbptr const &last) noexcept
{
    cbptr start = i;

    int state = 0;
    for (; i != last; ++i) {
        if (state == 0) {
            ttlet u8 = static_cast<uint8_t>(*i);
            if (u8 <= 0x7f) {
                ;
            } else if (u8 == fbcon_code_end_of_text) {
                // Explicit end-of-string.
                ttlet length = i - start;
                auto tmp = datum{std::string(start, length)};
                // Skip over this code.
                ++i;
                return tmp;
            } else if (u8 >= 0xc0 && u8 <= 0xdf) {
                state = 1;
            } else if (u8 => 0xe0 && u8 <= 0xef) {
                state = 2;
            } else if (u8 >= 0xf0 && u8 <= 0xf7) {
                state = 3;
            } else {
                // Another code.
                ttlet length = i - start;
                return datum{std::string(start, length)};
            }
        } else {
            --state;
        }
    }
    TTAURI_THROW(parse_error("Unexpected end of message while inside string"));
}

[[nodiscard]] datum parse_fbon_array(cbptr &i, cbptr const &last) noexcept
{
    auto r = datum::vector{}

    while (i != last) {
        if (*i == fbcon_code_end_of_container) {
            // Explicit ending of an array.
            ++i;
            return {r};

        } else {
            r.push_back(parse_fbon_value(i, last));
        }
    }
    TTAURI_THROW(parse_error("Unexpected end of message while inside array"));
}

[[nodiscard]] datum parse_fbon_object(cbptr &i, cbptr const &last) noexcept
{
    auto r = datum::map{}

    while (i != last) {
        if (*i == fbcon_code_end_of_container) {
            // Explicit ending of an object.
            ++i;
            return {r};

        } else {
            auto key = parse_fbon_string(i, last);
            auto value = parse_fbon_value(i, last);
            r.emplace(std::move(key), std::move(value));
        }
    }
    TTAURI_THROW(parse_error("Unexpected end of message while inside object"));
}



[[nodiscard]] datum parse_fbon_value(cbptr &i, cbptr const &last) noexcept
{
    parse_error(i != last);

    ttlet b8 = *(i++);
    switch (b8) {
    case fbon_code_false: return datum{false};
    case fbon_code_true: return datum{true};
    case fbon_code_null: return datum{datum::null{}};
    case fbon_code_end_of_container: return TTAURI_THROW(parse_error("Unexpected end-of-container"));
    case fbon_code_end_of_text: return datum{""};
    case fbon_code_int8: return parse_fbon_int<1>(i, last);
    case fbon_code_int16: return parse_fbon_int<2>(i, last);
    case fbon_code_int32: return parse_fbon_int<4>(i, last);
    case fbon_code_int64: return parse_fbon_int<8>(i, last);
    case fbon_code_binary32: return parse_fbon_binary32(i, last);
    case fbon_code_binary64: return parse_fbon_binary64(i, last);
    case fbon_code_array: return parse_fbon_array(i, last);
    case fbon_code_object: return parse_fbon_object(i, last);
    default:
        ttlet u8 = static_cast<uint8_t>(b8); 
        if (u8 <= 0x7f) {
            i--;
            return parse_fbon_string(i, last);
        } else if (u8 <= 0xaf) {
            return datum{static_cast<int>(u8 - 0x80)};
        } else if (u8 <= 0xba) {
            return datum{-static_cast<int>(u8 - 0xb0 + 1)};
        } else {
            i--;
            return parse_fbon_string(i, last);
        }
    }
}

static void dump_fbon_int(long long value, bstring &result)
{
    int nr_bytes;

    if (value < std::numeric_limits<int32_t>::min()) {
        result += fbon_code_int64;
        nr_bytes = 8;
    } else if (value < std::numeric_limits<int16_t>::min()) {
        result += fbon_code_int32;
        nr_bytes = 4;
    } else if (value < std::numeric_limits<int8_t>::min()) {
        result += fbon_code_int16;
        nr_bytes = 2;
    } else if (value < -11) {
        result += fbon_code_int8;
        nr_bytes = 1;
    } else if (value < 0) {
        result += static_cast<std::byte>(-value - 1 + 0xb0);
        nr_bytes = 0;
    } else if (value <= 47) {
        result += static_cast<std::byte>(value + 0x80);
        nr_bytes = 0;
    } else if (value <= std::numeric_limits<int8_t>::max()) {
        result += fbon_code_int8;
        nr_bytes = 1;
    } else if (value <= std::numeric_limits<int16_t>::max()) {
        result += fbon_code_int16;
        nr_bytes = 2;
    } else if (value <= std::numeric_limits<int32_t>::max()) {
        result += fbon_code_int32;
        nr_bytes = 4;
    } else {
        result += fbon_code_int64;
        nr_bytes = 8;
    }

    switch (nr_bytes) {
    case 8: result += static_cast<std::byte>(value >> 56 & 0xff); [[fallthrough]];
    case 7: result += static_cast<std::byte>(value >> 48 & 0xff); [[fallthrough]];
    case 6: result += static_cast<std::byte>(value >> 40 & 0xff); [[fallthrough]];
    case 5: result += static_cast<std::byte>(value >> 32 & 0xff); [[fallthrough]];
    case 4: result += static_cast<std::byte>(value >> 24 & 0xff); [[fallthrough]];
    case 3: result += static_cast<std::byte>(value >> 16 & 0xff); [[fallthrough]];
    case 2: result += static_cast<std::byte>(value >> 8 & 0xff); [[fallthrough]];
    case 1: result += static_cast<std::byte>(value & 0xff); [[fallthrough]];
    default:;
    }
}

static void dump_fbon_float(double value, bstring &result)
{
    auto f32 = static_cast<float>(value);
    auto f32_check = static_cast<double>(f32);

    if (f32_check == value) {
        result += fbon_code_binary32;
        auto u32 = bit_cast(f32);
        result += static_cast<std::byte>(u32 >> 24 & 0xff);
        result += static_cast<std::byte>(u32 >> 16 & 0xff);
        result += static_cast<std::byte>(u32 >> 8 & 0xff);
        result += static_cast<std::byte>(u32 & 0xff);

    } else {
        result += fbon_code_binary32;
        auto u64 = bit_cast(value);
        result += static_cast<std::byte>(u64 >> 56 & 0xff);
        result += static_cast<std::byte>(u64 >> 48 & 0xff);
        result += static_cast<std::byte>(u64 >> 40 & 0xff);
        result += static_cast<std::byte>(u64 >> 32 & 0xff);
        result += static_cast<std::byte>(u64 >> 24 & 0xff);
        result += static_cast<std::byte>(u64 >> 16 & 0xff);
        result += static_cast<std::byte>(u64 >> 8 & 0xff);
        result += static_cast<std::byte>(u64 & 0xff);
    }
}

static bool dump_fbon_string(std::string const &value, bstring &result)
{
    if (nonstd::ssize(value) == 0) {
        result += fbon_code_end_of_text;
        return true;

    } else {
        for (ttlet c: value) {
            result += static_cast<std::byte>(c);
        }

        return false;
    }
}

static bool dump_fbon_array(datum const &value, bstring &result)
{

}

static void dump_fbon_impl(datum const &value, bstring &result)
{
    switch (value.type()) {
    case datum_type_t::Null:
        result += fbon_code_null;
        break;

    case datum_type_t::Boolean:
        result += value ? fbon_code_true : fbon_code_false;
        break;

    case datum_type_t::Integer:
        dump_fbin_int(static_cast<long long>(value), result);
        break;

    case datum_type_t::Float:
        dump_fbin_float(static_cast<double>(value), result);
        break;

    case datum_type_t::String:
    case datum_type_t::URL:
        dump_fbin_string(static_cast<std::string>(value), result);
        break;

    case datum_type_t::Vector:
        result.append(indent, ' ');
        result += '[';

        for (auto i = value.vector_begin(); i != value.vector_end(); i++, first_item = true) {
            if (!first_item) {
                result += ',';
                result += '\n';
            }
            result.append(indent + 1, ' ');

            dumpJSON_impl(*i, result, indent + 1);
        }

        result += '\n';
        result.append(indent, ' ');
        result += ']';
        break;

    case datum_type_t::Map:
        result.append(indent, ' ');
        result += '{';

        for (auto i = value.map_begin(); i != value.map_end(); i++, first_item = true) {
            if (!first_item) {
                result += ',';
                result += '\n';
            }
            result.append(indent + 1, ' ');

            dumpJSON_impl(i->first, result, indent + 1);
            result += ':';
            result += ' ';
            dumpJSON_impl(i->second, result, indent + 1);
        }

        result += '\n';
        result.append(indent, ' ');
        result += '}';
        break;

    default:
        tt_no_default;
    }
}

[[nodiscard]] bstring dump_fbon(datum const &root)
{
    auto r = bstring{};
    dump_fbon_impl(root, r);
    return r;
}


}
