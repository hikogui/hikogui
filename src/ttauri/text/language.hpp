// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"
#include "../utils.hpp"
#include "../logger.hpp"
#include "../subsystem.hpp"
#include "../notifier.hpp"
#include "language_tag.hpp"
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace tt {


class language {
public:
    using callback_ptr_type = typename notifier<void()>::callback_ptr_type;

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
            r = plurality_func(narrow_cast<int>(n % 1'000'000));
        } else {
            // Use English as fallback.
            r = static_cast<int>(n == 1);
        }
        return std::clamp(narrow_cast<ssize_t>(r), ssize_t{0}, max - 1);
    }

    [[nodiscard]] static language *find(language_tag const &tag) noexcept {
        ttlet lock = std::scoped_lock(_mutex);

        ttlet i = _languages.find(tag);
        if (i != _languages.end()) {
            return i->second.get();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] static std::vector<language *> preferred_languages()
    {
        if (start_subsystem(_is_running, false, subsystem_init, subsystem_deinit)) {
            ttlet lock = std::scoped_lock(_mutex);
            return _preferred_languages;
        } else {
            return {};
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
        ttlet lock = std::scoped_lock(_mutex);

        auto tmp = std::vector<language*>{};
        for (ttlet &tag: add_short_names(tags)) {
            tmp.push_back(&find_or_create(tag));
        }

        if (compare_then_assign(_preferred_languages, tmp)) {
            auto language_order_string = std::string{};
            for (ttlet &language : tmp) {
                if (language_order_string.size() != 0) {
                    language_order_string += ", ";
                }
                language_order_string += to_string(language->tag);
            }
            tt_log_info("Setting preferred language in order: ", language_order_string);
        }
    }

    /** Get the preferred language tags from the operating system.
     * Language tags are based on IETF BCP-47/RFC-5646
     */
    [[nodiscard]] static std::vector<language_tag> read_os_preferred_languages() noexcept;

    [[nodiscard]] static callback_ptr_type subscribe(callback_ptr_type const &callback) noexcept
    {
        return _notifier.subscribe(callback);
    }

    template<typename Callback> requires(std::is_invocable_v<Callback>)
    [[nodiscard]] static callback_ptr_type subscribe(Callback &&callback) noexcept
    {
        return _notifier.subscribe(std::forward<Callback>(callback));
    }

    static void unsubscribe(callback_ptr_type const &callback) noexcept
    {
        _notifier.unsubscribe(callback);
    }

private:
    inline static std::atomic<bool> _is_running;
    inline static std::unordered_map<language_tag, std::unique_ptr<language>> _languages;
    inline static std::vector<language_tag> _preferred_language_tags;
    inline static std::vector<language *> _preferred_languages;
    inline static std::recursive_mutex _mutex;
    inline static notifier<void()> _notifier;
    inline static typename timer::callback_ptr_type _languages_maintenance_callback;

    [[nodiscard]] static bool subsystem_init() noexcept
    {
        _languages_maintenance_callback = timer::global().add_callback(1s, [](auto...) {
            ttlet new_preferred_language_tags = language::read_os_preferred_languages();

            if (language::_preferred_language_tags != new_preferred_language_tags) {
                language::_preferred_language_tags = new_preferred_language_tags;
                set_preferred_languages(language::_preferred_language_tags);
                language::_notifier();
            }
        }, true);

        return true;
    }

    static void subsystem_deinit() noexcept
    {
        if (_is_running.exchange(false)) {

        }
    }
};

}