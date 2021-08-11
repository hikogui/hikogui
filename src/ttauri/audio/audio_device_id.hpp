// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>

namespace tt {

class audio_device_id {
public:
    static constexpr char none = 0;
    static constexpr char win32 = 1;
    static constexpr char macos = 2;
    static constexpr char asio = 3;

    audio_device_id() noexcept {
        _v[0] = none;
        _v[1] = 0;
    }

    audio_device_id(char type, wchar_t const *id) noexcept;
    constexpr audio_device_id(audio_device_id const &) noexcept = default;
    constexpr audio_device_id(audio_device_id &&) noexcept = default;
    constexpr audio_device_id &operator=(audio_device_id const &) noexcept = default;
    constexpr audio_device_id &operator=(audio_device_id &&) noexcept = default;

    explicit operator bool () const noexcept
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
    std::array<char,64> _v;
};


}
