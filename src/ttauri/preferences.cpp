

#include "preferences.hpp"

namespace tt {

preferences::preferences(URL location) noexcept :
    _location(location), loading(false), modified(false)
{


}

preferences::~preferences()
{

}

void preferences::save()
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet tmp_location = _location + ".tmp";
    auto file = tt:file(tmp_location, access_mode::truncate_or_create_for_write);

    auto data = serialize();
    auto text = dumpJSON(data);
    file.write(text);
    file.flush();
    file.rename(_location, true);
}

void preferences::load()
{
    ttlet lock = std::scoped_lock(mutex);
    loading = true;

    auto file = tt::file(_location, access_mode::open_for_write);
    auto text = file.read_string();
    auto data = parseJSON(text);
    deserialize(data);

    loading = false;
}

}

