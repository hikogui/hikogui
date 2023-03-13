


def max_nr_bits(src):
    """Find the maximum number of bits needed to store any value in src.
    """
    return max(src).bit_length()


def bits_as_bytes(src, nr_bits):
    total_bits = nr_bits * len(src)
    total_bytes = (total_bits + 7) / 8

    as_long = 0
    for x in src:
        as_long <<= nr_bits
        as_long |= src

    # Align to the byte array.
    as_long <<= (total_bytes * 8 - total_bits) 

    dst = [0] * total_bytes
    for i in range(total_bytes):
        dst[i] = as_long & 0xff
        as_long >>= 8

    dst.reverse()
    return dst
