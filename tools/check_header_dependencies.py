

import sys

class module (object):
    def __init__(self, name):
        self.name = name
        self.dependencies = set()
        self.includes = set()

    def add_include(self, name):
        if name.endswith("/module.hpp"):
            module_name = name.split("/")[-2]
            self.dependencies.add(module_name)
        elif "win32_headers.hpp" in name:
            # This file is not included by utility/module.hpp
            pass
        elif "crt.hpp" in name:
            # Only files that have hi_main() defined may include this.
            pass
        elif "test" in name:
            # Some headers are only used in unit tests, they always have test in the name.
            pass
        elif "/" in name:
            self.includes.add(name)

    def __str__(self):
        r = "module: {}\n".format(self.name)

        if self.dependencies:
            r += " * dependecies:\n"
            for dependency in list(self.dependencies):
                r += "    {}\n".format(dependency)
            r += "\n"

        if self.includes:
            r += " * other includes:\n"
            for include in list(self.includes):
                r += "    {}\n".format(include)
            r += "\n"

        return r

def read_headers(filename, text, modules):
    module_name = filename.split("/")[-2]

    for line in text.split("\n"):
        line = line.strip()
        if line.startswith("#include \""):
            m = modules.setdefault(module_name, module(module_name))
            m.add_include(line[10:-1])

def main(filenames):
    modules = {}
    for filename in filenames:
        with open(filename, "r") as fd:
            try:
                read_headers(filename, fd.read(), modules)
            except Exception as e:
                print(e, file=sys.stderr)

    for module in modules.values():
        print(module)


if __name__ == "__main__":
    main(sys.argv[1:])

