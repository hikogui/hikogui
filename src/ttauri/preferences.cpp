// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "preferences.hpp"
#include "codec/JSON.hpp"
#include "file.hpp"
#include "timer.hpp"
#include "logger.hpp"

namespace tt {
namespace detail {

preference_item_base::preference_item_base(preferences &parent, std::string_view path) noexcept :
    _parent(parent), _path(path)
{
    _modified_callback_ptr = std::make_shared<std::function<void()>>([this]() {
        this->_parent.write(_path, this->serialize());
    });
}

void preference_item_base::load() const noexcept
{
    ttlet value = this->_parent.read(_path);
    if (value.is_undefined()) {
        this->reset();
    } else {
        this->deserialize(value);
    }
}

}

preferences::preferences() noexcept :
    _location(), _modified(false)
{
    using namespace std::literals::chrono_literals;

    _check_modified_ptr = timer::global().add_callback(5s, [this](auto...) {
        this->check_modified();
    });
}

preferences::preferences(URL location) noexcept :
    preferences()
{
    load(location);
}

preferences::~preferences()
{
    timer::global().remove_callback(_check_modified_ptr);
    save();
}

void preferences::_save() const noexcept
{
    try {
        auto text = format_JSON(data);

        ttlet tmp_location = _location.urlByAppendingExtension(".tmp");
        auto file = tt::file(tmp_location, access_mode::truncate_or_create_for_write | access_mode::rename);
        file.write(text);
        file.flush();
        file.rename(_location, true);

    } catch (io_error const &e) {
        tt_log_error("Could not save preferences to file. \"{}\"", e.what());
    }

    _modified = false;
}

void preferences::_load() noexcept
{
    try {
        reset();

        auto file = tt::file(_location, access_mode::open_for_read);
        auto text = file.read_string();
        data = parse_JSON(text);

        for (auto &item : items) {
            item->load();
        }

    } catch (io_error const &e) {
        tt_log_warning("Could not read preferences file. \"{}\"", e.what());

    } catch (parse_error const &e) {
        tt_log_error("Could not parse preferences file. \"{}\"", e.what());
    }
}

void preferences::reset() noexcept
{
    for (auto &item: items) {
        item->reset();
    }
}

void preferences::save(URL location) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    _location = std::move(location);
    _save();
}

void preferences::save() const noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    _save();
}

void preferences::load(URL location) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    _location = std::move(location);
    _load();
}

void preferences::load() noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    _load();
}

/** Write a value to the data.
 */
void preferences::write(jsonpath const &path, datum const value) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    auto *v = data.find_one_or_create(path);
    if (v == nullptr) {
        tt_log_fatal("Could not write '{}' to preference file '{}'", path, _location);
    }

    if (*v != value) {
        *v = value;
        _modified = true;
    }
}

/** Read a value from the data.
 */
datum preferences::read(jsonpath const &path) noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    if (auto *r = data.find_one(path)) {
        return *r;
    } else {
        return datum{std::monostate{}};
    }
}

void preferences::check_modified() noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    if (_modified) {
        save();
    }
}

}

