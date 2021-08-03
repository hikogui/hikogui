
#include <atomic>

namespace tt {

enum class global_state : uint64_t {
    log_level_debug = 0x01,
    log_level_info = 0x02,
    log_level_statistics = 0x04,
    log_level_trace = 0x08,
    log_level_audit = 0x10,
    log_level_warning = 0x20,
    log_level_error = 0x40,
    log_level_fatal = 0x80,

};

/** The global state of the ttauri framework.
 *
 * This variable contains state in use by multiple systems
 * inside ttauri. This is done so that this variable is likely
 * to be in a cache line and may stay in a register.
 *
 * In many cases using std::memory_order::relaxed loads are enough of a
 * guarantee to read this variable.
 */
inline std::atomic<global_state> global_state;

}

