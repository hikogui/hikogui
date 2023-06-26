


def bits_as_bytes(src):
    width = max(x.bit_length() for x in src)

    total_bits = width * len(src)
    total_bytes = (total_bits + 7) // 8

    as_long = 0
    for x in src:
        as_long <<= width
        as_long |= x

    # Align to the byte array.
    as_long <<= (total_bytes * 8 - total_bits)

    # Add 128 bits for large unaligned loads beyond the data.
    dst = [0] * (total_bytes + 16)
    for i in range(total_bytes - 1, -1, -1):
        dst[i] = as_long & 0xff
        as_long >>= 8

    return dst, width
