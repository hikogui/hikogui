// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system_delegate.hpp"
#include "audio_device.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../unique_or_borrow_ptr.hpp"
#include <vector>
#include <memory>

namespace tt {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class audio_system {
public:
    static inline unfair_recursive_mutex mutex;

    audio_system(unique_or_borrow_ptr<audio_system_delegate> delegate);
    virtual ~audio_system();
    audio_system(audio_system const &) = delete;
    audio_system(audio_system &&) = delete;
    audio_system &operator=(audio_system const &) = delete;
    audio_system &operator=(audio_system &&) = delete;

    [[nodiscard]] virtual std::vector<std::shared_ptr<audio_device>> devices() noexcept = 0;

    virtual void init() noexcept;
    virtual void deinit() noexcept;

    void set_delegate(unique_or_borrow_ptr<audio_system_delegate> delegate) noexcept
    {
        {
            ttlet lock = std::scoped_lock(audio_system::mutex);
            _delegate = std::move(delegate);
        }
        _delegate->audio_device_list_changed(*this);
    }

    audio_system_delegate &delegate() const noexcept
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);
        return *_delegate;
    }

    [[nodiscard]] static audio_system &global() noexcept
    {
        return *start_subsystem_or_terminate(_global, nullptr, subsystem_init, subsystem_deinit);
    }

protected:
    unique_or_borrow_ptr<audio_system_delegate> _delegate;

private:
    static inline std::atomic<audio_system *> _global;

    [[nodiscard]] static audio_system *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};

} // namespace tt