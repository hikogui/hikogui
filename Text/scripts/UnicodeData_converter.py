#!/usr/bin/env python3

import sys
import os.path

# Output Format
#
#
# struct {
#     uint32_t nr_codes
#
#  

class Unicode (object):
    def __init__(self):
        pass

    def __repr__(self):
        return "code={}, order={}, type={}, decomp={}".format(
            self.code,
            self.canonical_combining_class,
            self.decomposition_type,
            self.decomposition
        )

characters = {}

if len(sys.argv) != 2:
    print("Except one filename argument for the UnicodeData.txt")
    sys.exit(2)

input_filename = sys.argv[1]
input_fd = open(input_filename, encoding="utf-8")

for line in input_fd:
    line = line.rstrip()
    line = line.split("#", 1)[0]
    columns = [x.strip() for x in line.split(";")]

    char = Unicode()
    char.code = int(columns[0], 16)
    char.canonical_combining_class = int(columns[3])

    decomposition = columns[5]
    if not decomposition:
        decomposition = None
        decomposition_type = None
    else:
        if decomposition[0] == "<":
            decomposition_type, decomposition = decomposition[1:].split("> ")
        else:
            decomposition_type = None

        decomposition = [int(x, 16) for x in decomposition.split(" ")]

    char.decomposition = decomposition
    char.decomposition_type = decomposition_type

    characters[char.code] = char

for k,v in characters.items():
    #if v.canonical_combining_class > 0 and v.decomposition:
    #if v.decomposition:
    if v.code == 0xfb03:
        print(k, v)

print(len(characters))


