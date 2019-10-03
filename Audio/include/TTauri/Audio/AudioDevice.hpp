// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/bigint.hpp"
#include "TTauri/Audio/AudioDeviceDelegate.hpp"
#include "TTauri/Audio/AudioStreamConfig.hpp"
#include <boost/uuid/uuid.hpp>
#include <string>
#include <memory>

namespace TTauri::Audio {

/*! A set of audio channels which can be rendered and/or captures at the same time.
 * On win32 this would be Audio Endpoint Device, which can either render or capture
 * but not at the same time.
 *
 * On MacOS this would contain all the inputs and outputs of either a physical or
 * aggregate device that can run in the same clock domain, with both render and
 * capture at the same time.
 */
class AudioDevice {
private:
    std::shared_ptr<AudioDeviceDelegate> delegate = {};

public:
    std::string id;

    AudioDevice();
    virtual ~AudioDevice() = default;

    /*! Get a identfier for this device which can be stored
     * into a preferences file and be used after a reboot
     * to get the same device.
     */
    //virtual uuid uuid() const noexcept = 0;

    /*! Get a user friendly name of the audio device.
     * This is a combination of the name of the device and
     * the name of the end-point. Such as
     */
    virtual std::string name() const noexcept = 0;

    /*! Get a user friendly name of the audio device.
     * This is the name of the audio device itself, such as
     * "Realtek High Definition Audio".
     */
    virtual std::string deviceName() const noexcept = 0;

    /*! Get a user friendly name of the audio end-point device.
     * This is the name of the end point, such as "Microphone".
     */
    virtual std::string endPointName() const noexcept = 0;

    /*! Check if a audio configuration is supported by this device.
     * \param config Configuration such as sample rate, sample format and bit-depth.
     */
    //virtual bool isConfigSupported(AudioStreamConfig config) const noexcept = 0;

    /*! Start a session.
     * Start a session, which will cause data to be stream to and
     * from the audio device and the delegate's processAudio() function
     * to be called.
     *
     * This function may spawn a thread to handle the audio processing.
     * This function may throw an exception if it is not possible to start
     * a session.
     *
     * \param sessionId a unique ID used by the operating system to remember
     *        audio parameters for this stream, such as volume, accross reboots.
     * \param name A name used to by the operating system to display to the user
     *        when changing audio parameters through the operating system's preferences.
     * \param config Configuration such as sample rate, sample format and bit-depth.
     * XXX Windows allows for an icon to be passed to a session.
     */
    //virtual void startSession(boost::uuids::uuid sessionId, std::string name, AudioStreamConfig config) = 0;

    /*! Stop a session.
     * Stop a session, which will also stop the streams of audio.
     */
    //virtual void stopSession() noexcept = 0;
};

}