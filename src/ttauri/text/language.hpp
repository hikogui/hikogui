// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/observable.hpp"
#include "ttauri/numeric_cast.hpp"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace tt {

struct language {
    std::string tag;
    std::function<int(int)> plurality_func;

    language(std::string tag) noexcept;

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

    inline static std::unordered_map<std::string,std::unique_ptr<language>> languages;
    inline static observable<std::vector<language *>> preferred_languages;
    inline static std::recursive_mutex static_mutex;

    [[nodiscard]] static language *find(std::string const &tag) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);

        ttlet i = languages.find(tag);
        if (i != languages.end()) {
            return i->second.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] static language &find_or_create(std::string const &tag) noexcept {
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
    [[nodiscard]] static std::vector<std::string> add_short_names(std::vector<std::string> tags) noexcept {
        std::vector<std::string> r;

        std::string prev_short_tag;
        for (ttlet &tag: tags) {
            ttlet short_tag = split(tag, "-").front();

            if (ssize(prev_short_tag) != 0 && short_tag != prev_short_tag) {
                if (std::find(r.cbegin(), r.cend(), prev_short_tag) == r.cend()) {
                    r.push_back(prev_short_tag);
                }
            }

            if (std::find(r.cbegin(), r.cend(), tag) == r.cend()) {
                r.push_back(tag);
            }

            prev_short_tag = short_tag;
        }

        if (ssize(prev_short_tag) != 0) {
            if (std::find(r.cbegin(), r.cend(), prev_short_tag) == r.cend()) {
                r.push_back(prev_short_tag);
            }
        }
        return r;
    }

    static void set_preferred_languages(std::vector<std::string> tags) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);

        auto tmp = std::vector<language*>{};
        for (ttlet &tag: add_short_names(tags)) {
            tmp.push_back(&find_or_create(tag));
        }

        preferred_languages = tmp;
    }

    /** Get the preferred language tags from the operating system.
     * Language tags are based on IETF BCP-47/RFC-5646
     */
    [[nodiscard]] static std::vector<std::string> get_preferred_language_tags() noexcept;
};

}