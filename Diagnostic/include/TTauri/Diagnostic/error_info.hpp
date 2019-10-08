// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/string_tag.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Diagnostic/datum.hpp"
#include <array>
#include <tuple>

namespace TTauri {

class error_info_t {
    constexpr int max_keys = 8;

    inline std::array<std::pair<string_tag,datum>,max_keys> data;
    inline decltype(data::iterator) *end = nullptr;

    error_info_t() : end(data.begin()) {}
    ~error_info_t() = default;
    error_info_t(error_info_t const &) = delete;
    error_info_t(error_info_t &&) = delete;
    error_info_t &operator=(error_info_t const &) = delete;
    error_info_t &operator=(error_info_t &&) = delete;

    /*! Clear the error_info.
     * 
     */
    static inline void clear() noexcept {
        end = data.begin();
    }

    template<string_tag Tag>
    static inline void set(datum const &value) {
        values<Tag> = value;
        *(keys_end++) = Tag;
    }

    template<string_tag Tag>
    bool has() {
        for
    }

    template<string_tag Tag>
    static inline datum &get() {
        return values<Tag>;
    }
};

thread_local error_info_t error_info;

}
