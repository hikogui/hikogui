
print " | digits |   bits |   byte | bratio | Bratio |"
print " | ------:| ------:| ------:| ------:| ------:"
for digits in range(1, 31):
    value = 93 ** digits

    bits = 1
    while 2 ** bits <= value:
        bits+= 1
    bits-=1

    bytes = (bits ) / 8;
    parity_bits = bits % 8;

    print " | %6i | %6i | %6i | %6.2f | %6.2f |" % (digits, bits, bytes, float(bits) / digits, float(bytes*8) / digits)
