// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/sdatum.hpp"
#include "TTauri/Diagnositc/exceptions.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>

namespace TTauri {

sdatum::operator std::string() const noexcept {
    switch (type_id()) {
    case phy_boolean_id: return static_cast<bool>(*this) ? "true" : "false";
    case phy_null_id: return "null";
    case phy_undefined_id: return "undefined";
    case phy_integer_id0:
    case phy_integer_id1:
    case phy_integer_id2:
    case phy_integer_id3:
    case phy_integer_id4:
    case phy_integer_id5:
    case phy_integer_id6:
    case phy_integer_id7: return fmt::format("{}", static_cast<int64_t>(*this));
    case phy_string_id0:
    case phy_string_id1:
    case phy_string_id2:
    case phy_string_id3:
    case phy_string_id4:
    case phy_string_id5:
    case phy_string_id6: {
        let length = size();
        char buffer[6];
        for (int i = 0; i < length; i++) {
            buffer[i] = (u64 >> ((length - i - 1) * 8)) & 0xff;
        }
        return std::string(buffer, length);
    }
    default:
        if (is_phy_float()) {
            auto str = fmt::format("{:g}", static_cast<double>(*this));
            if (str.find('.') == str.npos) {
                str += ".0";
            }
            return str;
        } else {
            no_default;
        }
    }
}

char const *sdatum::type_name() const noexcept
{
    switch (type_id()) {
    case phy_boolean_id: return "Boolean";
    case phy_null_id: return "Null";
    case phy_undefined_id: return "Undefined";
    case phy_integer_id0:
    case phy_integer_id1:
    case phy_integer_id2:
    case phy_integer_id3:
    case phy_integer_id4:
    case phy_integer_id5:
    case phy_integer_id6:
    case phy_integer_id7:
    case phy_string_id0:
    case phy_string_id1:
    case phy_string_id2:
    case phy_string_id3:
    case phy_string_id4:
    case phy_string_id5:
    case phy_string_id6:
    default:
        if (ttauri_likely(is_phy_float())) {
            return "Float";
        } else {
            no_default;
        }
    }
}

std::string sdatum::repr() const noexcept
{
    switch (type_id()) {
    case phy_boolean_id: return static_cast<std::string>(*this);
    case phy_null_id: return static_cast<std::string>(*this);
    case phy_undefined_id: return static_cast<std::string>(*this);
    case phy_integer_id0:
    case phy_integer_id1:
    case phy_integer_id2:
    case phy_integer_id3:
    case phy_integer_id4:
    case phy_integer_id5:
    case phy_integer_id6:
    case phy_integer_id7:
    case phy_string_id0:
    case phy_string_id1:
    case phy_string_id2:
    case phy_string_id3:
    case phy_string_id4:
    case phy_string_id5:
    case phy_string_id6:
    default:
        if (ttauri_likely(is_phy_float())) {
            return static_cast<std::string>(*this);
        } else {
            no_default;
        }
    }
}

size_t sdatum::size() const noexcept
{
    switch (type_id()) {
    case phy_string_id0:
    case phy_string_id1:
    case phy_string_id2:
    case phy_string_id3:
    case phy_string_id4:
    case phy_string_id5:
    case phy_string_id6: return to_int(((u64 & 0xffff'0000'0000'0000ULL) - string_mask) >> 48);
    default: no_default;
    }
}

std::string to_string(sdatum const &d) noexcept
{
    return static_cast<std::string>(d);
}

std::ostream &operator<<(std::ostream &os, sdatum const &d)
{
    return os << static_cast<std::string>(d);
}





}
