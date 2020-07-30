
#include "../byte_string.hpp"
#include "../required.hpp"
#include "../datum.hpp"
#include "../exceptions.hpp"
#include <cstddef>

#pragma once

namespace tt {

constexpr auto BON8_code_float_min_one = std::byte{0xba};
constexpr auto BON8_code_float_zero    = std::byte{0xbb};
constexpr auto BON8_code_float_one     = std::byte{0xbc};
constexpr auto BON8_code_array_empty   = std::byte{0xbd};
constexpr auto BON8_code_object_empty  = std::byte{0xbe};
constexpr auto BON8_code_null          = std::byte{0xbf};
constexpr auto BON8_code_bool_false    = std::byte{0xc0};
constexpr auto BON8_code_bool_true     = std::byte{0xc1};
constexpr auto BON8_code_int32         = std::byte{0xf8};
constexpr auto BON8_code_int64         = std::byte{0xf9};
constexpr auto BON8_code_binary32      = std::byte{0xfa};
constexpr auto BON8_code_binary64      = std::byte{0xfb};
constexpr auto BON8_code_array         = std::byte{0xfc};
constexpr auto BON8_code_object        = std::byte{0xfd};
constexpr auto BON8_code_eoc           = std::byte{0xfe};
constexpr auto BON8_code_eot           = std::byte{0xff};

class BON8_encoder {
    bool open_string;
    bstring output;

public:
    BON8_encoder() noexcept :
        open_string(false), output() {}

    bstring const &get() const noexcept {
        return output;
    }

    void add(signed long long value) noexcept {
        open_string = false;

        if (value < std::numeric_limits<int32_t>::min()) {
            output += BON8_code_int64;
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(value >> (56 - i*8));
            }

        } else if (value < -33554432) {
            output += BON8_code_int32;
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(value >> (24 - i*8));
            }

        } else if (value < -262144) {
            value = -value - 1;
            output += static_cast<std::byte>(0xf0 + (value >> 22 & 0x07));
            output += static_cast<std::byte>(0xc0 + (value >> 16 & 0x3f));
            output += static_cast<std::byte>(value >> 8);
            output += static_cast<std::byte>(value);

        } else if (value < -1920) {
            value = -value - 1;
            output += static_cast<std::byte>(0xe0 + (value >> 14 & 0x0f));
            output += static_cast<std::byte>(0xc0 + (value >> 8 & 0x3f));
            output += static_cast<std::byte>(value);

        } else if (value < -10) {
            value = -value - 1;
            output += static_cast<std::byte>(0xc2 + (value >> 6 & 0x1f));
            output += static_cast<std::byte>(0xc0 + (value & 0x3f));

        } else if (value < 0) {
            value = -value - 1;
            output += static_cast<std::byte>(0xb0 + value);

        } else if (value <= 47) {
            output += static_cast<std::byte>(0x80 + value);

        } else if (value <= 3839) {
            output += static_cast<std::byte>(0xc2 + (value >> 7 & 0x1f));
            output += static_cast<std::byte>(value & 0x7f);

        } else if (value <= 524287) {
            output += static_cast<std::byte>(0xe0 + (value >> 15 & 0x0f));
            output += static_cast<std::byte>(value >> 8 & 0x7f);
            output += static_cast<std::byte>(value);

        } else if (value <= 67108863) {
            output += static_cast<std::byte>(0xf0 + (value >> 23 & 0x17));
            output += static_cast<std::byte>(value >> 16 & 0x7f);
            output += static_cast<std::byte>(value >> 8);
            output += static_cast<std::byte>(value);

        } else if (value <= std::numeric_limits<int32_t>::max()) {
            output += BON8_code_int32;
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(value >> (24 - i*8));
            }

        } else {
            output += BON8_code_int64;
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(value >> (56 - i*8));
            }
        }
    }

    void add(unsigned long long value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(signed long value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(unsigned long value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(signed int value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(unsigned int value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(signed short value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(unsigned short value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(signed char value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(unsigned char value) noexcept {
        return add(numeric_cast<signed long long>(value));
    }

    void add(double value) noexcept {
        open_string = false;

        ttlet f32 = static_cast<float>(value);
        ttlet f32_64 = static_cast<double>(f32);

        if (value == -1.0) {
            output += BON8_code_float_min_one;

        } else if (value == 0.0 || value == -0.0) {
            output += BON8_code_float_zero;

        } else if (value == 1.0) {
            output += BON8_code_float_one;

        } else if (f32_64 == value) {
            uint32_t u32;
            std::memcpy(&u32, &f32, sizeof(u32));

            output += BON8_code_binary32;
            for (int i = 0; i != 4; ++i) {
                output += static_cast<std::byte>(u32 >> (24 - i*8));
            }

        } else {
            uint64_t u64;
            std::memcpy(&u64, &value, sizeof(u64));

            output += BON8_code_binary64;
            for (int i = 0; i != 8; ++i) {
                output += static_cast<std::byte>(u64 >> (56 - i*8));
            }
        }
    }

    void add(float value) noexcept {
        return add(numeric_cast<double>(value));
    }

    void add(bool value) noexcept {
        open_string = false;
        output += value ? BON8_code_bool_true : BON8_code_bool_false;
    }

    void add(nullptr_t value) noexcept {
        open_string = false;
        output += BON8_code_null;
    }

    void add(std::string_view value) noexcept {
        if (open_string) {
            output += BON8_code_eot;
        }

        if (nonstd::ssize(value) == 0) {
            output += BON8_code_eot;
            open_string = false;

        } else {
            int multi_byte = 0;

            for (ttlet _c : value) {
                ttlet c = static_cast<uint8_t>(_c);

                if constexpr (BuildType::current == BuildType::Debug) {
                    if (multi_byte == 0) {
                        if (c >= 0xc2 && c <= 0xdf) {
                            multi_byte = 1;
                        } else if (c >= 0xe0 && c <= 0xef) {
                            multi_byte = 2;
                        } else if (c >= 0xf0 && c <= 0xf7) {
                            multi_byte = 3;
                        } else {
                            tt_assert(c <= 0x7f);
                        }

                    } else {
                        tt_assert(c >= 0x80 && c <= 0xbf);
                        --multi_byte;
                    }
                }

                output += static_cast<std::byte>(c);
            }

            tt_assert(multi_byte == 0);

            open_string = true;
        }
    }

    void add(std::string const &value) noexcept {
        return add(std::string_view{value});
    }

    void add(char *value) noexcept {
        return add(std::string_view{value});
    }

    void add(datum const &value);

    template<typename T>
    void add(std::vector<T> const &items) {
        open_string = false;
        if (nonstd::ssize(items) == 0) {
            output += BON8_code_array_empty;
        } else {
            output += BON8_code_array;

            for (ttlet &item: items) {
                add(item);
            }

            output += BON8_code_eoc;
        }
        open_string = false;
    }

    template<typename Key, typename Value>
    void add(std::map<Key,Value> const &items) {
        using item_type = std::map<Key,Value>::value_type;

        open_string = false;
        if (nonstd::ssize(value) == 0) {
            output += BON8_code_object_empty;
        } else {
            // Keys must be ordered lexically.
            auto sorted_items = std::vector<std::reference_wrapper<item_type>>{items.begin(), items.end()};
            std::sort(sorted_items.begin(), sorted_value.end(), [](item_type const &a, item_type const &b) {
                return
                    static_cast<std::string_view>(a.first) <
                    static_cast<std::string_view>(b.first);
            });

            output += BON8_code_object;
            for (item_type const &item: sorted_value) {
                add(static_cast<std::string_view>(item.first));
                add(item.second);
            }
            output += BON8_code_eoc;
        }
        open_string = false;
    }
};

void BON8_encoder::add(datum const &value) {
    if (value.is_string() || value.is_url()) {
        add(static_cast<std::string>(value));
    } else if (value.is_bool()) {
        add(static_cast<bool>(value));
    } else if (value.is_null()) {
        add(nullptr);
    } else if (value.is_integer()) {
        add(static_cast<signed long long>(value));
    } else if (value.is_float()) {
        add(static_cast<double>(value));
    } else if (value.is_vector()) {
        add(static_cast<datum::vector>(value));
    } else if (value.is_map()) {
        add(static_cast<datum::map>(value));
    } else {
        TTAURI_THROW(invalid_operation_error("Datum value can not be encoded to BON8"));
    }
}


}