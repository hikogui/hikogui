#!/bin/sh python3

import sys
import os
import os.path
from SourceCode import SourceCode


def main():
    if not os.path.exists("src"):
        print("Expect 'src' directory in the current working directory.")
        sys.exit()

    macro_headers = [
        os.path.join("src", "hikogui", "crt.hpp"),
        os.path.join("src", "hikogui", "macros.hpp"),
        os.path.join("src", "hikogui", "test.hpp"),
        os.path.join("src", "hikogui", "win32_headers.hpp")
    ]

    sources = []
    for root, dirnames, filenames in os.walk("src"):
        for filename in filenames:
            header_path = os.path.join(root, filename)
            if header_path in macro_headers:
                continue

            basename, extension = os.path.splitext(filename)
            if extension != ".hpp":
                continue

            assert (root.startswith("src"))
            module_root = os.path.join("mod", root[4:])
            module_path = os.path.join(module_root, basename + ".ixx")

            print("Parsing file {}".format(header_path))
            sources.append(SourceCode(header_path, module_path))


if __name__ == "__main__":
    main()
