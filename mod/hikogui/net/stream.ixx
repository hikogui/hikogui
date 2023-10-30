// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_net_stream;

export namespace hi::inline v1 {

class Socketstream {
protected:
    bool connecting = false;

    /** Buffer with data read from the socket.
     */
    packet_buffer readBuffer;

    /** Buffer with data to write.
     */
    packet_buffer writeBuffer;

public:
    /** Handle connected event.
     */
    void handleConnect();

    /** Check if the socket-stream needs to read.
     */
    virtual bool needToRead();

    /** Check if the socket-stream needs to read.
     */
    virtual bool needToWrite();

    /** Handle ready-to-read event.
     *
     */
    [[nodiscard]] virtual void handleReadyToReadEvent();

    /** Handle ready-to-write event.
     *
     */
    [[nodiscard]] virtual void handleReadyToWriteEvent();
}

} // namespace hi::inline v1
