// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <typeinfo>
#include <filesystem>

export module hikogui_settings_preferences;
import hikogui_codec;
import hikogui_dispatch;
import hikogui_file;
import hikogui_observer;
import hikogui_telemetry;

export namespace hi::inline v1 {
class preferences;

namespace detail {

class preference_item_base {
public:
    preference_item_base(preferences& parent, std::string_view path) noexcept : _parent(parent), _path(path) {}

    preference_item_base(preference_item_base const&) = delete;
    preference_item_base(preference_item_base&&) = delete;
    preference_item_base& operator=(preference_item_base const&) = delete;
    preference_item_base& operator=(preference_item_base&&) = delete;
    virtual ~preference_item_base() = default;

    /** Reset the value.
     */
    virtual void reset() noexcept = 0;

    /** Load a value from the preferences.
     */
    void load() noexcept;

protected:
    preferences& _parent;
    jsonpath _path;

    /** Encode the value into a datum.
     *
     * @return A datum representing the value, or undefined if same as the initial value.
     */
    [[nodiscard]] virtual datum encode() const noexcept = 0;

    virtual void decode(datum const& data) = 0;
};

template<typename T>
class preference_item : public preference_item_base {
public:
    preference_item(preferences& parent, std::string_view path, observer<T> const& value, T init) noexcept :
        preference_item_base(parent, path), _value(value), _init(std::move(init))
    {
        _value_cbt = _value.subscribe(
            [this](auto...) {
                if (auto tmp = this->encode(); not holds_alternative<std::monostate>(tmp)) {
                    this->_parent.write(_path, this->encode());
                } else {
                    this->_parent.remove(_path);
                }
            },
            callback_flags::local);
    }

    void reset() noexcept override
    {
        _value = _init;
    }

protected:
    [[nodiscard]] datum encode() const noexcept override
    {
        if (*_value != _init) {
            return hi::pickle<T>{}.encode(*_value);
        } else {
            return datum{std::monostate{}};
        }
    }

    void decode(datum const& data) override
    {
        _value = hi::pickle<T>{}.decode(data);
    }

private:
    T _init;
    observer<T> _value;
    callback<void(T)> _value_cbt;
};

} // namespace detail

/** user preferences.
 *
 * A preferences objects maintains a link between observer in the application and
 * a preferences file.
 *
 * When loading preferences the observer are set to the value
 * in the preferences file. When an observer changes a value the preferences file is
 * updated to reflect this change. For performance reasons multiple modifications are
 * combined into a single save.
 *
 * An application may open multiple preferences files, for example an application preferences file
 * and a project-specific preferences file. The name of the project-specific preferences file
 * can then be selected by the user.
 *
 * The preferences file is updated by using the operating system specific call to
 * overwrite an existing file atomically.
 */
class preferences {
public:
    /** Mutex used to synchronize changes to the preferences.
     *
     * This mutex may be used externally to atomically combine multiple observer modification
     * into a single change of the preferences file.
     */
    mutable std::mutex mutex;

    /** Construct a preferences instance.
     *
     * No current preferences file will be selected.
     *
     * It is recommended to call `preferences::load(std::filesystem::path)` after the constructor.
     */
    preferences() noexcept : _location(), _data(datum::make_map()), _modified(false)
    {
        using namespace std::chrono_literals;

        _check_modified_cbt = loop::timer().repeat_function(5s, [this](auto...) {
            this->check_modified();
        });
    }

    /** Construct a preferences instance.
     *
     * The current preferences file is changed to the location give.
     *
     * @param location The location of the preferences file to load from.
     */
    preferences(std::filesystem::path location) noexcept : preferences()
    {
        load(location);
    }

    preferences(std::string_view location) : preferences(std::filesystem::path{location}) {}
    preferences(std::string const& location) : preferences(std::filesystem::path{location}) {}
    preferences(char const *location) : preferences(std::filesystem::path{location}) {}

    ~preferences()
    {
        save();
    }

    preferences(preferences const&) = delete;
    preferences(preferences&&) = delete;
    preferences& operator=(preferences const&) = delete;
    preferences& operator=(preferences&&) = delete;

    /** Save the preferences.
     *
     * This will load the preferences from the current selected file.
     */
    void save() const noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        _save();
    }

    /** Save the preferences.
     *
     * This will change the current preferences file to the location given.
     *
     * @param location The file to save the preferences to.
     */
    void save(std::filesystem::path location) noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        _location = std::move(location);
        _save();
    }

    /** Load the preferences.
     *
     * This will load the preferences from the current selected file.
     */
    void load() noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        _load();
    }

    /** Load the preferences.
     *
     * This will change the current preferences file to the location given.
     *
     * @param location The file to save the preferences to.
     */
    void load(std::filesystem::path location) noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        _location = std::move(location);
        _load();
    }

    /** Reset data members to their default value.
     */
    void reset() noexcept
    {
        _data = datum::make_map();
        for (auto& item : _items) {
            item->reset();
        }
    }
    /** Register an observer to a preferences file.
     *
     * @param path The json-path inside the preference file.
     * @param item The observer to monitor.
     * @param init The value of the observer when it is not present in the preferences file.
     */
    template<typename T>
    void add(std::string_view path, observer<T> const& item, T init = T{}) noexcept
    {
        auto item_ = std::make_unique<detail::preference_item<T>>(*this, path, item, std::move(init));
        item_->load();
        _items.push_back(std::move(item_));
    }

private:
    /** The location of the preferences file.
     */
    std::filesystem::path _location;

    /** The data from the preferences file.
     */
    datum _data;

    /** The data was modified.
     * When this flag is true the preferences should be saved.
     */
    mutable bool _modified = false;

    /** List of registered items.
     */
    std::vector<std::unique_ptr<detail::preference_item_base>> _items;

    callback<void()> _check_modified_cbt;

    void _load() noexcept
    {
        try {
            auto file = hi::file(_location, access_mode::open_for_read);
            auto text = file.read_string();
            _data = parse_JSON(text);

            for (auto& item : _items) {
                item->load();
            }

        } catch (io_error const& e) {
            hi_log_warning("Could not read preferences file. \"{}\"", e.what());
            reset();

        } catch (parse_error const& e) {
            hi_log_error("Could not parse preferences file. \"{}\"", e.what());
            reset();
        }
    }

    void _save() const noexcept
    {
        try {
            auto text = format_JSON(_data);

            auto tmp_location = _location;
            tmp_location += ".tmp";

            auto file = hi::file(tmp_location, access_mode::truncate_or_create_for_write | access_mode::rename);
            file.write(text);
            file.flush();
            file.rename(_location, true);

        } catch (io_error const& e) {
            hi_log_error("Could not save preferences to file. \"{}\"", e.what());
        }

        _modified = false;
    }

    /** Check if there are modification in data and save when necessary.
     */
    void check_modified() noexcept
    {
        hilet lock = std::scoped_lock(mutex);

        if (_modified) {
            _save();
        }
    }

    /** Write a value to the data.
     */
    void write(jsonpath const& path, datum const value) noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        auto *v = _data.find_one_or_create(path);
        if (v == nullptr) {
            hi_log_fatal("Could not write '{}' to preference file '{}'", path, _location.string());
        }

        if (*v != value) {
            *v = value;
            _modified = true;
        }
    }

    /** Read a value from the data.
     */
    datum read(jsonpath const& path) noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        if (auto const *const r = _data.find_one(path)) {
            return *r;
        } else {
            return datum{std::monostate{}};
        }
    }

    /** Remove a value from the data.
     */
    void remove(jsonpath const& path) noexcept
    {
        hilet lock = std::scoped_lock(mutex);
        if (_data.remove(path)) {
            _modified = true;
        }
    }

    friend class detail::preference_item_base;
    template<typename T>
    friend class detail::preference_item;
};

void detail::preference_item_base::load() noexcept
{
    hilet value = this->_parent.read(_path);
    if (value.is_undefined()) {
        this->reset();
    } else {
        try {
            this->decode(value);
        } catch (std::exception const&) {
            hi_log_error("Could not decode preference {}, value {}", _path, value);
            this->reset();
        }
    }
}

} // namespace hi::inline v1
