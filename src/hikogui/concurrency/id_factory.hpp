

#include "fixed_string.hpp"
#include "../macros.hpp"

namespace hi { inline namespace v1 {

/** A factory for unique IDs.
 *
 * This factory allows you to release an ID so that it may be reused at a later time.
 *
 * Acquiring a new ID
 *
 */
template<std::unsigned_integral T>
class id_factory_t {
public:
    using value_type = T;

    id_factory_t() = default;
    id_factory_t(id_factory_t const &) = delete;
    id_factory_t(id_factory_t &&) = delete;
    id_factory_t &operator=(id_factory_t const &) = delete;
    id_factory_t &operator=(id_factory_t &&) = delete;
  
    /** Get the next ID.
     *
     * @note This algorithm is wait-free.
     * @return A new ID, or a previously released ID.
     */ 
    [[nodiscard]] value_type acquire() noexcept
    {
        auto expected = _released_count.load(std::memory_order::relaxed);
        if (expected != 0) {
            // There MAY be released IDs.

            if (_released_count.compare_exchange_weak(expected, expected - 1, std::memory_order::acquire, std::memory_order::relaxed)) {
                // There ARE released IDs.

                // unfair_mutex::try_lock() is wait-free.
                if (_mutex.try_lock()) {
                    // We got a lock, pop the last ID.
                    hi_axiom(not _released.empty());

                    // std::vector::pop_back() should be wait-free since there are no reallocations.
                    hilet tmp = _released.back();
                    _released.pop_back();

                    _mutex.unlock();
                    return tmp;

                } else {
                    // We couldn't get a lock; access to release-stack may be slow due to reallocation.
                    // Since we didn't pop, increment the release-count again.
                    _released_count.fetch_add(1, std::memory_order::relaxed);
                }
            }
        }

        return _v.fetch_add(1, std::memory_order::relaxed) + 1;
    }

    /** Release an ID for reuse.
     *
     * If you want to release from a real-time thread you may post this on the
     * main thread.
     *
     * @note This algorithm is blocking.
     * @param v The ID to release
     */
    void release(value_type v) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        _released.push_back(v);
        _released_count.fetch_add(1, std::memory_order::release);
    }

    /** @sa acquire()
     */
    [[nodiscard]] value_type operator++() noexcept
    {
        return acquire();
    }

private:
    mutable unfair_mutex _mutex = {};
    std::atomic<value_type> _v = 0;

    std::atomic<size_t> _released_count = 0;
    std::vector<value_type> _released = {};
};

template<std::unsigned_integral T, fixed_string>
inline auto id_factory_t = id_factory_t<T>;



}}

