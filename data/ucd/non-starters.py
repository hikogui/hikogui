#!/usr/bin/env python3

ranges = []
nr_non_starters = 0
for line in open("UnicodeData.txt"):
    columns = line.strip().split(";")
    if len(columns) != 15:
        continue

    code_point = int(columns[0], 16)
    canonical_combining_class = int(columns[3], 10)
    if canonical_combining_class != 0:
        nr_non_starters += 1
        if not ranges or (ranges[-1][1] + 1) != code_point:
            ranges.append((code_point, code_point))
        else:
            first, last = ranges[-1]
            ranges[-1] = (first, code_point)

for range_ in ranges:
    print(range_)

print(nr_non_starters)

