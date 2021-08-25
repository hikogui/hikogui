// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../codec/pickle.hpp"
#include "../exception.hpp"
#include "../check.hpp"
#include <array>

namespace tt {

class audio_device_id {
public:
    static constexpr char none = 0;
    static constexpr char win32 = 1;
    static constexpr char macos = 2;
    static constexpr char asio = 3;

    audio_device_id() noexcept
    {
        _v[0] = none;
        _v[1] = 0;
    }

    audio_device_id(char type, wchar_t const *id) noexcept;
    constexpr audio_device_id(audio_device_id const &) noexcept = default;
    constexpr audio_device_id(audio_device_id &&) noexcept = default;
    constexpr audio_device_id &operator=(audio_device_id const &) noexcept = default;
    constexpr audio_device_id &operator=(audio_device_id &&) noexcept = default;

    explicit operator bool() const noexcept
    {
        return _v[0] != none;
    }

    [[nodiscard]] friend bool operator==(audio_device_id const &lhs, audio_device_id const &rhs) noexcept
    {
        for (size_t i = 0; i != std::size(lhs._v); ++i) {
            if (lhs._v[i] != rhs._v[i]) {
                return false;
            }
            if (lhs._v[i] == 0 or rhs._v[i] == 0) {
                break;
            }
        }
        return true;
    }

private:
    std::array<char, 64> _v;

    friend struct pickle<audio_device_id>;
};

template<>
struct pickle<audio_device_id> {
    [[nodiscard]] std::string encode(audio_device_id const &rhs) const noexcept
    {
        auto r = std::string{};

        auto t = rhs._v[0];
        if (t == audio_device_id::none) {
            return r;
        } else if (t == audio_device_id::win32) {
            r += 'w';
            for (auto i = 1_uz; i != std::size(rhs._v); ++i) {
                if (auto c = rhs._v[i]) {
                    r += c;
                } else {
                    break;
                }
            }
            return r;

        } else {
            tt_not_implemented();
        }
    }

    [[nodiscard]] audio_device_id decode(std::string_view rhs) const
    {
        auto r = audio_device_id{};
        if (rhs.empty()) {
            return r;

        } else {
            auto t = rhs[0];
            if (t == 'w') {
                tt_parse_check(std::size(rhs) <= std::size(r._v), "win32-audio_device_id pickle size to large {}", rhs);

                r._v[0] = audio_device_id::win32;
                auto i = 1_uz;
                for (; i != std::size(rhs); ++i) {
                    r._v[i] = rhs[i];
                }
                if (i != std::size(r._v)) {
                    r._v[i] = 0;
                }
                return r;

            } else {
                throw parse_error("audio_device_id pickle unknown type {}", t);
            }
        }
    }
};

} // namespace tt
