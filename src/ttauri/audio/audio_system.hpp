// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system_delegate.hpp"
#include "audio_device.hpp"
#include <vector>
#include <memory>

namespace tt {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class audio_system {
public:
    audio_system(std::weak_ptr<audio_system_delegate> const &delegate);
    virtual ~audio_system();
    audio_system(audio_system const &) = delete;
    audio_system(audio_system &&) = delete;
    audio_system &operator=(audio_system const &) = delete;
    audio_system &operator=(audio_system &&) = delete;

    [[nodiscard]] virtual std::vector<std::shared_ptr<audio_device>> devices() noexcept = 0;

    virtual void init() noexcept;

protected:
    std::weak_ptr<audio_system_delegate> _delegate;

public:
    static inline unfair_recursive_mutex mutex;
    static inline std::shared_ptr<audio_system> global;
};

}