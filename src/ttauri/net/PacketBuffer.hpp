

#pragma once

namespace tt {

class PacketBuffer {
    std::list<Packet> packets;
    ssize_t _totalNrBytes;
    bool _closed;

public:
    /** Connection is closed.
     * @return true when the connection has be closed.
     */
    bool closed() const noexcept {
        return _closed;
    }

    /** Total number of bytes in the buffer.
     */
    ssize_t nrBytes() const noexcept {
        return _totalNrBytes;
    }

    /** Total number of packets in the buffer.
     * This contains the number of messages on a message based socket.
     * On a stream based socket this number is not useful, but larger
     * than zero when data is available.
     */
    ssize_t nrPackets() const noexcept {
        return packets.size();
    }

    /** Close the connection on this side.
     */
    void close() noexcept {
        _closed = true;
    }

    /** Get a new packet to write a message into.
     * @return a pointer to an byte array with at least nrBytes of data available.
     */
    nonstd::span<std::byte> getNewPacket(ssize_t nrBytes) noexcept {
        tt_assert(!closed());
        packets.emplace_back(nrBytes);
        return {packets.back().end(), nrBytes};
    }
   
    /** Get a packet to write a stream of bytes into.
     * @return a pointer to an byte array with at least nrBytes of data available.
     */
    nonstd::span<std::byte> getPacket(ssize_t nrBytes) noexcept {
        tt_assert(!closed());
        if (packets.empty() || (packets.back().writeSize() < nrBytes)) {
            packets.emplace_back(nrBytes);
        }
        return {packets.back().end(), nrBytes};
    }
   
    /** Write the data added to the packet.
     * This function will write the data added into the buffers returned
     * by `getNewPacket()` and `getPacket()`.
     *
     * @param nrByted The number of bytes written into the packet.
     * @param push Push the data through the socket, bypass Nagel algorithm.
     */
    void write(ssize_t nrBytes, bool push=true) noexcept {
        tt_assert(!closed());
        packets.back().write(nrBytes);
        if (push) {
            packets.back().push();
        }
        _totalNrBytes += nrBytes;
    }

    /** Peek into the data without consuming.
     * @param nrBytes the minimum amount of data required.
     * @return empty if not enough bytes available; otherwise the data.
     *         The returned size may be larger than requested and
     *         this data may be consumed using `read()`.
     */
    nonstd::span<std::byte const> peek(ssize_t nrBytes) {
        if (packets.empty() || size() < nrBytes) {
            return {};
        }

        while (true) {
            if (packets.front().readSize() >= nrBytes) {
                return {packets.front().readSize(), std::ssize(packets)};
            }

            // Check if we can merge packets.
            tt_assert(packets.front().size() >= nrBytes);

            // Merge data from next packet.
        }
    }

    /** Peek into the data a single text-line without consuming.
     * Throws parse_error when the line is longer than nrBytes.
     *
     * @param nrBytes maximum line size
     * @return empty if there are no lines; otherwise a line of data.
     *         The line-feed or nul is included at the end of the string.
     */
    std::string_view peekLine(ssize_t nrBytes=1024) {
        ssize_t packetNr = 0;
        ssize_t byteNr = 0;
        ssize_t i = 0;
        while (packetNr < nrPackets()) {
            parse_assert(byteNr < nrBytes, "New-line not found within {} bytes", nrBytes);

            if (i == std::ssize(packets[packetNr])) {
                // Advance to next packet.
                ++packetNr;
                i = 0;
            }

            ttlet c = packets[packetNr][i]
            if (c == '\n' || c == '\0') {
                // Found end-of-line
                ttlet bspan = peek(byteNr + 1);
                return {reinterpret_cast<char *>(bspan.data(), byteNr + 1};
            }
            ++i;
            ++byteNr;
        }

        // Not enough bytes read yet.
        return {}
    }

    /** Consume the data from the buffer.
     * This function will consume the data read using `peek()` and
     * `peekLine()`.
     *
     * @param nrByted The number of bytes to consume.
     */
    void read(ssize_t nrBytes) noexcept {
        peekBuffer.clear();

        while (nrBytes) {
            ttlet packet_size = std::ssize(packets.front());
            if (nrBytes >= packet_size) {
                packets.pop_front();
            } else {
                packets.front().read(nrBytes);
                tt_assume(std::ssize(packets.front()) > 0);
            }
            nrBytes -= std::ssize(packets_size);
        }
    }


};


}

