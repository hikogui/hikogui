


def max_num_bits(src):
    """Find the maximum number of bits needed to store any value in src.
    """
    return max(src).bit_length()


def bits_as_bytes(src, nr_bits):
    total_bits = nr_bits * len(src)
    total_bytes = (total_bits + 7) // 8

    as_long = 0
    for x in src:
        as_long <<= nr_bits
        as_long |= x

    # Align to the byte array.
    as_long <<= (total_bytes * 8 - total_bits)

    # Add 128 bits for large unaligned loads beyond the data.
    dst = [0] * (total_bytes + 16)
    for i in range(total_bytes - 1, -1, -1):
        dst[i] = as_long & 0xff
        as_long >>= 8

    return dst
