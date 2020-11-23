// Copyright 2020 Pokitec
// All rights reserved.

#include "preferences.hpp"
#include "encoding/JSON.hpp"
#include "file.hpp"

namespace tt {

preferences::preferences(URL location) noexcept :
    _location(location), loading(false), modified(false)
{
    _set_modified_ptr = std::make_shared<std::function<void()>>([this]() {
        this->set_modified();
    });
}

preferences::~preferences()
{
}

void preferences::save() const noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet tmp_location = _location.urlByAppendingExtension(".tmp");

    auto text = dumpJSON(serialize());

    try {
        auto file = tt::file(tmp_location, access_mode::truncate_or_create_for_write | access_mode::rename);
        file.write(text);
        file.flush();
        file.rename(_location, true);

    } catch (io_error &e) {
        LOG_ERROR("Could not save preferences to file: {}", to_string(e));
    }
}

void preferences::load() noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    loading = true;

    reset();
    
    try {
        auto file = tt::file(_location, access_mode::open_for_read);
        auto text = file.read_string();
        auto data = parseJSON(text);
        deserialize(data);
        loading = false;

    } catch (io_error &e) {
        LOG_WARNING("Could not read preferences file: {}", to_string(e));
        loading = false;

    } catch (parse_error &e) {
        LOG_ERROR("Could not parse preferences file: {}", to_string(e));
        loading = false;
    }
}

}

