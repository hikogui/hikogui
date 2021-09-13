// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "datum.hpp"
#include "timer.hpp"
#include "logger.hpp"
#include <typeinfo>

#pragma once

namespace tt {
namespace detail {

class preference_item_base {
public:
    preference_item_base(std::string_view path) noexcept :
        _path(parse_jsonpath(path)) {}

    preference_item_base(preference_item_base const &) = delete;
    preference_item_base(preference_item_base &&) = delete;
    preference_item_base &operator=(preference_item_base const &) = delete;
    preference_item_base &operator=(preference_item_base &&) = delete;
    virtual ~preference_item_base() = default;

    virtual void reset() const noexcept = 0;

    virtual void serialize(datum &data) const noexcept = 0;

    virtual void deserialize(data const &data) const noexcept = 0;

protected:
    jsonpath _path;
};


template<typename T>
class preference_item : public preference_item_base {
public:
    preference_item(preferences &self, std::string_view path, observable<T> &value, T &&init) noexcept :
        preference_item_base(path), _value(value), _init(std::forward<T>(init))
    {
        _value.subscribe(self._set_modified_ptr);
    }

    virtual void reset() const noexcept
    {
        _value = _init;
    }

    virtual void serialize(datum &data) const noexcept
    {
        data.put(_path, *_value);
    }

    virtual void deserialize(data const &data) const noexcept
    {
        _value = data.get(_path, _init);
    }

private:
    T _init;
    observable<T> &_value;
};


}

/** user preferences.
 * User preferences are kept in an instance of the preferences subclass.
 *
 * This class will automatically load preferences from the location given in its
 * constructor and save the preferences when they are modified.
 *
 * The saving if preferences is delayed to combine multiple modification into a single
 * save. By locking the mutex external to the preferences multiple modifications can
 * be stored atomically.
 */
class preferences {
public:
    mutable std::recursive_mutex mutex;

    preferences(URL location) noexcept;
    virtual ~preferences();
    preferences(preferences const &) = delete;
    preferences(preferences &&) = delete;
    preferences &operator=(preferences const &) = delete;
    preferences &operator=(preferences &&) = delete;

    /** Save the preferences.
     */
    void save() const noexcept;

    /** Load the preferences.
     */
    void load() noexcept;

    /** Reset data members to their default value.
     */
    [[nodiscard]] virtual void reset() noexcept
    {
        for (ttlet &item: _items) {
            item->reset();
        }
    }

    /** Serialize the preferences data.
     * The serialize method is called when the preferences need to be saved.
     * A subclass should implement serialize() to serialize all the members
     * of its class.
     *
     * It is recommended to call super::serialize() from subclass implementations.
     */
    [[nodiscard]] virtual datum serialize() const noexcept
    {
        auto r = datum::make_map();

        for (ttlet &item: _items) {
            item->serialize(r);
        }

        return r;
    }

    /** Deserialize the preferences.
     * The deserialize method is called when the preferences are loaded.
     * A subclass should implement the deserialize() to read the data and
     * rebuild its members. It should ignore any data it does not know about.
     *
     * It is recommended to call super::deserialize() from subclass implementations.
     */
    virtual void deserialize(datum const &data) noexcept
    {
        for (ttlet &item: _items) {
            item->deserialize(data);
        }
    }

protected:
    /** The function pointer which will call the data_modified() method.
     * This pointer should be used to subscribe the preferences to
     * each of its observable members.
     */
    std::shared_ptr<std::function<void()>> _set_modified_ptr;

    /** Preferences are loading.
     * During deserializing this flag should be incremented
     * to prevent reactions to modification of the data.
     */
    int _deserializing = 0;

    template<typename T, typename Observable, typename Key>
    static void deserialize_value(Observable &obj, datum const &data, Key const &key) noexcept
    {
        if (data.contains(key)) {
            auto const &data_value = data[key];
            if (holds_alternative<T>(data_value)) {
                obj = static_cast<T>(data_value);
            } else {
                tt_log_error(
                    "Could not deserialize '{}' for key '{}' to a '{}' type.",
                    to_string(data_value),
                    key,
                    typeid(T).name());
            }
        }
    }

    template<typename T>
    static void register_item(std::string_view key, observable<T> &item, T &&init = T{}) noexcept
    {
        items.push_back(std::make_unique<preference_item<T>>(*this, key, item, std::forward<T>(init));
    }

private:
    URL _location;

    /** List of registered items.
     */
    std::vector<std::unique_ptr<detail::preference_item_base>> items;

    /** The data was modified.
     * When this flag is true the preferences should be saved.
     */
    mutable bool _modified = false;

    timer::callback_ptr_type _check_modified_ptr;

    /** This function is called whenever data is modified.
     */
    void set_modified() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_axiom(_deserializing >= 0);
        if (_deserializing == 0) {
            _modified = true;
        }
    }

    void check_modified() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_axiom(_deserializing >= 0);

        if (_modified) {
            save();
        }
    }
};

} // namespace tt
