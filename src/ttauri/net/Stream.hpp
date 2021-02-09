// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

namespace tt {

class SocketStream {

protected:
    bool connecting = false;

    /** Buffer with data read from the socket.
     */
    PacketBuffer readBuffer;

    /** Buffer with data to write.
     */
    PacketBuffer writeBuffer;

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



}

