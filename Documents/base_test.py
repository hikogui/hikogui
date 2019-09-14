

for base in range(2, 95):

    best_ratio = 0.0
    best_chars = None
    best_digits = None
    for chars in range(1, 72):
        value = base ** chars

        digits = 1
        while (2 ** digits) <= value:
            digits+= 1

        digits-= 1
        ratio = float(digits) / float(chars)

        if ratio > best_ratio:
            best_ratio = ratio
            best_chars = chars
            best_digits = digits

    print "base %i chars %i digits %i ratio %f" % (base, best_chars, best_digits, best_ratio)
