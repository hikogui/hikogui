

bit_list = [19, 17, 16, 18, 26, 24, 22, 21, 23, 25]


value = 5808
for bit in bit_list:
    new_value = value + 2 ** bit
    print(value, new_value-1)
    value = new_value

