// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "datum.hpp"
#include "timer.hpp"
#include "logger.hpp"
#include "jsonpath.hpp"
#include "observable.hpp"
#include "codec/pickle.hpp"
#include <typeinfo>

#pragma once

namespace tt {
class preferences;

namespace detail {

class preference_item_base {
public:
    preference_item_base(preferences &parent, std::string_view path) noexcept;

    preference_item_base(preference_item_base const &) = delete;
    preference_item_base(preference_item_base &&) = delete;
    preference_item_base &operator=(preference_item_base const &) = delete;
    preference_item_base &operator=(preference_item_base &&) = delete;
    virtual ~preference_item_base() = default;

    /** Reset the value.
     */
    virtual void reset() const noexcept = 0;

    /** Load a value from the preferences.
     */
    void load() const noexcept;

protected:
    std::shared_ptr<std::function<void()>> _modified_callback_ptr;
    preferences &_parent;
    jsonpath _path;

    virtual datum serialize() const noexcept = 0;

    virtual void deserialize(datum const &data) const noexcept = 0;
};

template<typename T>
class preference_item : public preference_item_base {
public:
    preference_item(preferences &parent, std::string_view path, observable<T> &value, T &&init) noexcept :
        preference_item_base(parent, path), _value(value), _init(std::forward<T>(init))
    {
        _value.subscribe(this->_modified_callback_ptr);
    }

    virtual void reset() const noexcept
    {
        _value = _init;
    }

protected:
    virtual datum serialize() const noexcept
    {
        return tt::pickle<T>{}.serialize(*_value);
    }

    virtual void deserialize(datum const &data) noexcept
    {
        _value = tt::pickle<T>{}.deserialize(data);
    }

private:
    T _init;
    observable<T> &_value;
};

} // namespace detail

/** user preferences.
 *
 * The saving if preferences is delayed to combine multiple modification into a single
 * save. By locking the mutex external to the preferences multiple modifications can
 * be stored atomically.
 */
class preferences {
public:
    mutable std::recursive_mutex mutex;

    /** Construct a preferences instance.
     *
     * No current preferences file will be selected.
     *
     * It is recommended to call `preferences::load(URL)` after the constructor.
     */
    preferences() noexcept;

    /** Construct a preferences instance.
     *
     * The current preferences file is changed to the location give.
     *
     * @param location The location of the preferences file to load from.
     */
    preferences(URL location) noexcept;

    ~preferences();
    preferences(preferences const &) = delete;
    preferences(preferences &&) = delete;
    preferences &operator=(preferences const &) = delete;
    preferences &operator=(preferences &&) = delete;

    /** Save the preferences.
     *
     * This will load the preferences from the current selected file.
     */
    void save() const noexcept;

    /** Save the preferences.
     *
     * This will change the current preferences file to the location given.
     *
     * @param location The file to save the preferences to.
     */
    void save(URL location) noexcept;

    /** Load the preferences.
     *
     * This will load the preferences from the current selected file.
     */
    void load() noexcept;

    /** Load the preferences.
     *
     * This will change the current preferences file to the location given.
     *
     * @param location The file to save the preferences to.
     */
    void load(URL location) noexcept;

    /** Reset data members to their default value.
     */
    [[nodiscard]] void reset() noexcept;

    /** Write a value to the data.
     */
    void write(jsonpath const &path, datum const value) noexcept;

    /** Read a value from the data.
     */
    datum read(jsonpath const &path) noexcept;

    /** Register an observable to a preferences file.
    * 
    * The lifetime of @a item must extent beyond the last call to
    * `preferences::load()` or `preferences::reset()`.
    * 
    * XXX Modify observable to be able to get a weak_ptr to the internals of observable
    * to check-or-extent the lifetime of the observable.
    * 
    * @param path The json-path inside the preference file.
    * @param item The observable to monitor.
    * @param init The value of the observable when it is not present in the preferences file.
    */
    template<typename T>
    static void register_item(std::string_view path, observable<T> &item, T &&init = T{}) noexcept
    {
        auto item = std::make_unique<preference_item<T>>(*this, path, item, std::forward<T>(init));
        item->load();
        items.push_back(std::move(item));
    }

private:
    URL _location;

    /** The data from the preferences file.
     */
    datum data;

    /** The data was modified.
     * When this flag is true the preferences should be saved.
     */
    mutable bool _modified = false;

    timer::callback_ptr_type _check_modified_ptr;

    /** List of registered items.
     */
    std::vector<std::unique_ptr<detail::preference_item_base>> items;

    void _load() noexcept;
    void _save() const noexcept;

    void check_modified() noexcept;
};

} // namespace tt
