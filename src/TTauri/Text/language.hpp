// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/observable.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace tt {

struct language {
    std::string name;
    std::function<int(int)> plurality_func;

    language(std::string name) noexcept :
        name(std::move(name)), plurality_func() {}

    [[nodiscard]] int plurality(long long n, int max) const noexcept {
        int r;
        if (plurality_func) {
            r = plurality_func(numeric_cast<int>(n % 1'000'000));
        } else {
            // Use English as fallback.
            r = static_cast<int>(n != 1);
        }
        return std::clamp(r, 0, max - 1);
    }

    inline static std::unordered_map<std::string,std::unique_ptr<language>> languages;
    inline static std::vector<language *> prefered_languages;
    inline static std::recursive_mutex static_mutex;

    [[nodiscard]] static language *find(std::string const &name) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);

        ttlet i = languages.find(name);
        if (i != languages.end()) {
            return i->second.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] static language *find_or_create(std::string const &name) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);

        auto *r = find(name);
        if (!r) {
            auto tmp = std::make_unique<language>(name);
            r = tmp.get();
            languages[name] = std::move(tmp);
        }
        return r;
    }

    /** Add short language names to the list of names.
     * The short names are inserted right after a consecutive group of long names with the same short name.
     */
    [[nodiscard]] static std::vector<std::string> add_short_names(std::vector<std::string> names) noexcept {
        std::vector<std::string> r;

        std::string prev_short_name;
        for (ttlet &name: names) {
            ttlet split_name = split(name, "-");
            tt_assume(ssize(split_name) != 0);
            ttlet short_name = split_name.front();

            if (ssize(prev_short_name) != 0 && short_name != prev_short_name) {
                if (std::find(r.cbegin(), r.cend(), prev_short_name) == r.cend()) {
                    r.push_back(prev_short_name);
                }
            }

            if (std::find(r.cbegin(), r.cend(), name) == r.cend()) {
                r.push_back(name);
            }

            prev_short_name = short_name;
        }

        if (ssize(prev_short_name) != 0) {
            if (std::find(r.cbegin(), r.cend(), prev_short_name) == r.cend()) {
                r.push_back(prev_short_name);
            }
        }
        return r;
    }

    static void set_prefered_languages(std::vector<std::string> names) noexcept {
        ttlet lock = std::scoped_lock(static_mutex);
        prefered_languages.clear();

        for (ttlet &name: add_short_names(names)) {
            prefered_languages.push_back(find_or_create(name));
        }
    }
};

inline observable<std::vector<std::string>> language_list = std::vector<std::string>{"en-US"};

[[nodiscard]] std::vector<std::string> read_os_language_list() noexcept; 


}