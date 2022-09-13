// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_device.hpp"
#include "audio_stream_format.hpp"
#include "audio_format_range.hpp"
#include "../generator.hpp"

struct IMMDevice;
struct IPropertyStore;
struct IMMEndpoint;
struct IAudioEndpointVolume;
struct IAudioMeterInformation;
struct IAudioClient;

namespace hi::inline v1 {

/*! A class representing an audio device on the system.
 */
class audio_device_win32 : public audio_device {
public:
    audio_device_win32(IMMDevice *device);
    ~audio_device_win32();

    /** Get the device id for the given win32 audio end-point.
     *
     * @param device The win32 device instance to get the device id from.
     * @return A device id as string, to differentiate with asio it will start with "win32:"
     * @throws io_error When the device id could not be retrieved.
     */
    [[nodiscard]] static std::string get_device_id(IMMDevice *device);

    /// @privatesection
    void update_state() noexcept override;
    [[nodiscard]] hi::label label() const noexcept override;
    [[nodiscard]] audio_device_state state() const noexcept override;
    [[nodiscard]] audio_direction direction() const noexcept override;
    [[nodiscard]] bool exclusive() const noexcept override;
    void set_exclusive(bool exclusive) noexcept override;
    [[nodiscard]] double sample_rate() const noexcept override;
    void set_sample_rate(double sample_rate) noexcept override;
    [[nodiscard]] hi::speaker_mapping input_speaker_mapping() const noexcept override;
    void set_input_speaker_mapping(hi::speaker_mapping speaker_mapping) noexcept override;
    [[nodiscard]] std::vector<hi::speaker_mapping> available_input_speaker_mappings() const noexcept override;
    [[nodiscard]] hi::speaker_mapping output_speaker_mapping() const noexcept override;
    void set_output_speaker_mapping(hi::speaker_mapping speaker_mapping) noexcept override;
    [[nodiscard]] std::vector<hi::speaker_mapping> available_output_speaker_mappings() const noexcept override;
    [[nodiscard]] bool supports_format(audio_stream_format const& format) const noexcept;
    /// @endprivatesection
private:
    std::string _end_point_id;
    audio_device_state _previous_state;
    audio_direction _direction;
    bool _exclusive = false;
    double _sample_rate = 0.0;
    hi::speaker_mapping _speaker_mapping = hi::speaker_mapping::none;
    audio_stream_format _current_stream_format;

    IMMDevice *_device = nullptr;
    IMMEndpoint *_end_point = nullptr;
    IPropertyStore *_property_store = nullptr;
    IAudioClient *_audio_client = nullptr;

    [[nodiscard]] std::string end_point_name() const noexcept;

    /** Get a user friendly name of the audio device.
     * This is the name of the audio device itself, such as
     * "Realtek High Definition Audio".
     */
    [[nodiscard]] std::string device_name() const noexcept;

    /** Get a user friendly description of the audio end-point device.
     * This is the name of the end point, such as "Microphone".
     */
    [[nodiscard]] std::string end_point_description() const noexcept;

    GUID pin_category() const noexcept;

    /** Get the legacy waveIn or waveOut id.
     *
     * The waveIn/waveOut id are required to get access to the audio device driver
     * to query the formats that it supports.
     */
    unsigned int wave_id() const;

    /** Query the audio device through the driver to determine the supported formats.
     */
    void update_supported_formats() noexcept;

    /** Find a stream format based on the prototype_stream_format.
     *
     * This function looks for a supported stream format when the device is used in exclusive-mode.
     * The prototype consists of a sample-rate and speaker mapping. From this the best sample format
     * is selected: int24 -> float32 -> int20 -> int16. int24 is selected above float, so that hikogui
     * will handle dithering.
     *
     * @param sample_rate The sample rate selected by the user.
     * @param speaker_mapping The speaker mapping selected by the user.
     * @return A supported audio_stream_format or empty.
     */
    [[nodiscard]] audio_stream_format
    find_exclusive_stream_format(double sample_rate, hi::speaker_mapping speaker_mapping) noexcept;

    /** Get the shared stream format for the device.
     */
    [[nodiscard]] audio_stream_format shared_stream_format() const;

    [[nodiscard]] generator<audio_format_range> get_format_ranges() const noexcept;
};

} // namespace hi::inline v1
