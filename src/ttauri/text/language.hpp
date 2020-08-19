// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../observable.hpp"
#include "../numeric_cast.hpp"
#include "language_tag.hpp"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace tt {


struct language {
    language_tag tag;
    std::function<int(int)> plurality_func;

    language(language_tag tag) noexcept;

    language(language const &) = delete;
    language(language &&) = delete;
    language &operator=(language const &) = delete;
    language &operator=(language &&) = delete;

    [[nodiscard]] ssize_t plurality(long long n, ssize_t max) const noexcept {
        int r;
        if (plurality_func) {
            r = plurality_func(numeric_cast<int>(n % 1'000'000));
        } else {
            // Use English as fallback.
            r = static_cast<int>(n == 1);
        }
        return std::clamp(numeric_cast<ssize_t>(r), ssize_t{0}, max - 1);
    }

    inline static std::unordered_map<language_tag,std::unique_ptr<language>> languages;
    inline static observable<std::vector<language *>> preferred_languages;
    inline static std::recursive_mutex static_mutex;

    [[nodiscard]] static language *find(language_tag const &tag) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);

        ttlet i = languages.find(tag);
        if (i != languages.end()) {
            return i->second.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] static language &find_or_create(language_tag const &tag) noexcept
    {
        ttlet lock = std::scoped_lock(static_mutex);

        auto *r = find(tag);
        if (!r) {
            auto tmp = std::make_unique<language>(tag);
            r = tmp.get();
            languages[tag] = std::move(tmp);
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
        for (ttlet &tag: tags) {
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

    static void set_preferred_languages(std::vector<language_tag> tags) noexcept
    {
        ttlet lock = std::scoped_lock(static_mutex);

        auto tmp = std::vector<language*>{};
        for (ttlet &tag: add_short_names(tags)) {
            tmp.push_back(&find_or_create(tag));
        }

        auto language_order_string = std::string{};
        for (ttlet &language : tmp) {
            if (language_order_string.size() != 0) {
                language_order_string += ", ";
            }
            language_order_string += to_string(language->tag);
        }
        LOG_INFO("Setting preferred language in order: ", language_order_string);

        preferred_languages = tmp;
    }

    /** Get the preferred language tags from the operating system.
     * Language tags are based on IETF BCP-47/RFC-5646
     */
    [[nodiscard]] static std::vector<language_tag> get_preferred_language_tags() noexcept;
};

}