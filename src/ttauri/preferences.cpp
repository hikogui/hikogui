// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "preferences.hpp"
#include "codec/JSON.hpp"
#include "file.hpp"
#include "timer.hpp"
#include "logger.hpp"

namespace tt {

preferences::preferences(URL location) noexcept :
    _location(location), _deserializing(0), _modified(false)
{
    _set_modified_ptr = std::make_shared<std::function<void()>>([this]() {
        this->set_modified();
    });

    _check_modified_ptr = timer::global->add_callback(5s, [this](auto...) {
        this->check_modified();
    });
}

preferences::~preferences()
{
}

void preferences::save() const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet tmp_location = _location.urlByAppendingExtension(".tmp");

    auto text = format_JSON(serialize());

    try {
        auto file = tt::file(tmp_location, access_mode::truncate_or_create_for_write | access_mode::rename);
        file.write(text);
        file.flush();
        file.rename(_location, true);

    } catch (io_error const &e) {
        tt_log_error("Could not save preferences to file. \"{}\"", e.what());
    }

    _modified = false;
}

void preferences::load() noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    ++_deserializing;

    reset();
    
    try {
        auto file = tt::file(_location, access_mode::open_for_read);
        auto text = file.read_string();
        auto data = parse_JSON(text);
        deserialize(data);
        --_deserializing;

    } catch (io_error const &e) {
        tt_log_warning("Could not read preferences file. \"{}\"", e.what());
        --_deserializing = false;

    } catch (parse_error const &e) {
        tt_log_error("Could not parse preferences file. \"{}\"", e.what());
        --_deserializing = false;
    }
}

}

