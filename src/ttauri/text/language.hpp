// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"
#include "../utils.hpp"
#include "language_tag.hpp"
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <format>

namespace tt::inline v1 {

class language {
public:
    language_tag tag;
    std::function<int(int)> plurality_func;

    language(language_tag tag) noexcept;
    language(language const &) = delete;
    language(language &&) = delete;
    language &operator=(language const &) = delete;
    language &operator=(language &&) = delete;

    [[nodiscard]] ssize_t plurality(long long n, ssize_t max) const noexcept
    {
        int r;
        if (plurality_func) {
            r = plurality_func(narrow_cast<int>(n % 1'000'000));
        } else {
            // Use English as fallback.
            r = static_cast<int>(n == 1);
        }
        return std::clamp(narrow_cast<ssize_t>(r), ssize_t{0}, max - 1);
    }

    [[nodiscard]] static language *find(language_tag const &tag) noexcept
    {
        ttlet lock = std::scoped_lock(_mutex);

        ttlet i = _languages.find(tag);
        if (i != _languages.end()) {
            return i->second.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] static language &find_or_create(language_tag const &tag) noexcept
    {
        ttlet lock = std::scoped_lock(_mutex);

        auto *r = find(tag);
        if (!r) {
            auto tmp = std::make_unique<language>(tag);
            r = tmp.get();
            _languages[tag] = std::move(tmp);
        }
        return *r;
    }

    /** Add short language names to the list of names.
     * The short names are inserted right after a consecutive group of long names with the same short name.
     */
    [[nodiscard]] static std::vector<language_tag> add_short_names(std::vector<language_tag> tags) noexcept
    {
        std::vector<language_tag> r;

        auto prev_short_tag = language_tag{};
        for (ttlet &tag : tags) {
            ttlet short_tag = tag.short_tag();

            if (prev_short_tag && short_tag != prev_short_tag) {
                if (std::find(r.cbegin(), r.cend(), prev_short_tag) == r.cend()) {
                    r.push_back(prev_short_tag);
                }
            }

            if (std::find(r.cbegin(), r.cend(), tag) == r.cend()) {
                r.push_back(tag);
            }

            prev_short_tag = short_tag;
        }

        if (prev_short_tag) {
            if (std::find(r.cbegin(), r.cend(), prev_short_tag) == r.cend()) {
                r.push_back(prev_short_tag);
            }
        }
        return r;
    }

    [[nodiscard]] static std::vector<language *> make_languages(std::vector<language_tag> tags) noexcept
    {
        ttlet lock = std::scoped_lock(_mutex);

        auto r = std::vector<language *>{};
        for (ttlet &tag : add_short_names(tags)) {
            r.push_back(&find_or_create(tag));
        }
        return r;
    }

private:
    inline static std::atomic<bool> _is_running;
    inline static std::unordered_map<language_tag, std::unique_ptr<language>> _languages;
    inline static std::recursive_mutex _mutex;
};

} // namespace tt::inline v1

template<typename CharT>
struct std::formatter<std::vector<tt::language *>, CharT> : std::formatter<std::string_view, CharT> {
    auto format(std::vector<tt::language *> const &t, auto &fc)
    {
        auto r = std::string{};
        for (ttlet language : t) {
            if (not r.empty()) {
                r += ", ";
            }
            r += to_string(language->tag);
        }
        return std::formatter<std::string_view, CharT>::format(r, fc);
    }
};
