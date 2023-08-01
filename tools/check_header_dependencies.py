

import sys
import os
import pathlib

class module (object):
    def __init__(self, name):
        self.name = name
        self.dependencies = set()
        self.includes = set()

    def add_include(self, path):
        dir_name, file_name = os.path.split(path)
        basedir, module_name = os.path.split(dir_name)

        if file_name == "module.hpp" or file_name == module_name + ".hpp":
            if module_name:
                self.dependencies.add(module_name)

        elif file_name in ("win32_headers.hpp", "macros.hpp", "crt.hpp"):
            # These files are not part of any module.
            pass

        elif dir_name:
            self.includes.add(path)

    def check_loop(self, modules, visited):
        for dependency_name in self.dependencies:
            if dependency_name in visited:
                # Found a loop
                return dependency_name

            dependency = modules[dependency_name]
            loop_path = dependency.check_loop(modules, visited | set([dependency_name]))
            if loop_path:
                return "{} -> {}".format(dependency_name, loop_path)

        return None

    def __str__(self):
        r = ""
        for dependency in list(self.dependencies):
            r += "{} -> {}\n".format(self.name, dependency)

        for include in list(self.includes):
            r += "{} -> {}\n".format(self.name, include)

        return r

def read_headers(filename, text, modules):
    module_name = filename.split("/")[-2]

    for line in text.split("\n"):
        line = line.strip()
        if line.startswith("#include \""):
            end = line.rfind("\"")
            m = modules.setdefault(module_name, module(module_name))
            m.add_include(line[10:end])


def main(filenames):
    modules = {}
    for filename in filenames:
        with open(filename, "r") as fd:
            try:
                read_headers(filename, fd.read(), modules)
            except Exception as e:
                print(e, file=sys.stderr)

    # Find loops
    found_loop = False
    for module in modules.values():
        loop_path = module.check_loop(modules, set([module.name]))
        if loop_path is not None:
            found_loop = True
            print("Found circular dependency: {}".format(loop_path))
    if found_loop:
        sys.exit()

    print("digraph G {\n")
    for module in modules.values():
        print(module)
    print("}\n")

if __name__ == "__main__":
    filenames_to_parse = []

    for top, directories, filenames in os.walk("."):
        top = pathlib.PureWindowsPath(top).as_posix()

        if top == ".":
            # Don't include files from the top-level directory as those are
            # not modules.
            continue

        for filename in filenames:
            if "test" in filename:
                # Do not include unit-tests.
                continue

            filenames_to_parse.append("{}/{}".format(top, filename))

    main(filenames_to_parse)

