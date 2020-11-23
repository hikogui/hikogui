
#include "URL.hpp"
#include "datum.hpp"

#pragma once

namespace tt {

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

    preferences(URL location);
    virtual ~preferences();
    preferences(preferences const &) = delete;
    preferences(preferences &&) = delete;
    preferences &operator=(preferences const &) = delete;
    preferences &operator=(preferences &&) = delete;

    /** Reset data members to their default value.
     */
    [[nodiscard]] virtual void reset() noexcept
    {
    }

    /** Save the preferences.
     */
    void save();

    /** Load the preferences.
     */
    void load();

    /** Serialize the preferences data.
     * The serialize method is called when the preferences need to be saved.
     * A subclass should implement serialize() to serialize all the members
     * of its class.
     *
     * It is recommended to call super::serialize() from subclass implementations.
     */
    [[nodiscard]] virtual datum serialize() noexcept
    {
        return {datum::object{}};
    }

    /** Deserialize the preferences.
     * The deserialize method is called when the preferences are loade.
     * A subclass should implement the deserialize() to read the data and
     * rebuild its members. It should ignore any data it does not know about.
     *
     * It is recommended to call super::deserialize() from subclass implementations.
     */
    virtual void deserialize(datum const &data) noexcept
    {
    }

protected:
    /** The function pointer which will call the data_modified() method.
     * This pointer should be used to subscribe the preferences to
     * each of its observable members.
     */
    std::shared_ptr<std::function<void()> set_modified_ptr;

private:
    URL _location;

    /** Preferences are loading.
     * During loading, deserializing and resetting this flag should be set to true
     * to prevent reactions to modification of the data.
     */
    bool loading;

    /** The data was modified.
     * When this flag is true the preferences should be saved.
     */
    bool modified;

    /** This function is called whenever data is modified.
     */
    void set_modified() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        if (!loading) {
            modified = true;
        }
    }

public:
    static std::unique_ptr<preferences> global;
};


}
