
for chars in range(1, 14):
    value = 93 ** chars

    bits = 1
    while 2 ** bits <= value:
        bits+= 1
    bits-=1

    bytes = bits / 8;
    parity_bits = bits % 8;

    print "%i characters -> %i bits -> %i data + %i parity" % (chars, bits, bytes * 8, parity_bits)
