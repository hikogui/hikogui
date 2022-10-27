// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

/** A network message or stream buffer.
 */
class packet {
    std::byte *data;
    std::byte *data_end;
    std::byte *first;
    std::byte *last;
    bool _pushed = false;

public:
    /** Allocate an empty packet of a certain size.
     */
    packet(ssize_t nrBytes) noexcept
    {
        data = new std::byte[nrBytes];
        data_end = data + nrBytes;
        first = data;
        last = data;
    }

    ~packet() noexcept
    {
        delete[] data;
    }

    packet(packet const &rhs) noexcept = delete;
    packet operator=(packet const &rhs) noexcept = delete;

    packet(packet &&rhs) noexcept : data(rhs.data), data_end(rhs.data_end), first(rhs.first), last(rhs.last)
    {
        rhs.data = nullptr;
    }

    [[nodiscard]] std::byte *begin() noexcept
    {
        return first;
    }

    [[nodiscard]] std::byte *end() noexcept
    {
        return last;
    }

    /** How many bytes can be read from this buffer.
     */
    [[nodiscard]] ssize_t readSize() const noexcept
    {
        return last - first;
    }

    /** How many bytes can still be written to this buffer.
     */
    [[nodiscard]] ssize_t writeSize() const noexcept
    {
        return data_end - last;
    }

    /** Should this packet be pushed onto the network.
     */
    [[nodiscard]] bool pushed() const noexcept
    {
        return _pushed;
    }

    /** Mark this packet to be pushed to the network.
     */
    void push() noexcept
    {
        _pushed = true;
    }

    /** Commit a write.
     * Should be called after data has been copied into this buffer.
     */
    void write(ssize_t nrBytes) noexcept
    {
        last += nrBytes;
        hi_assert(last <= data_end);
    }

    /** Consume a read.
     * Should be called after data has been copied from this buffer.
     */
    void read(ssize_t nrBytes) noexcept
    {
        first += nrBytes;
        hi_assert(first <= last);
    }
};

} // namespace hi::inline v1
