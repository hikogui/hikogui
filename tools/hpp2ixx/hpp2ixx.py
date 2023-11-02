#!/bin/sh python3

import sys
import os
import os.path
from SourceCode import SourceCode
    
def replace_top(path, old_top, new_top):
    path = os.path.relpath(path, old_top)
    return os.path.join(new_top, path)

def parse_header_files(source_dir, destination_dir, macro_headers):
    sources = []
    rest = []
    for root, dirnames, filenames in os.walk(source_dir):
        assert (root.startswith(source_dir))
        module_root = replace_top(root, source_dir, destination_dir)

        for filename in filenames:
            header_path = os.path.join(root, filename)
            if header_path in macro_headers:
                rest.append((header_path, os.path.join(module_root, filename)))
                continue

            if filename.endswith(".hpp.in"):
                module_path = os.path.join(module_root, filename[:-7] + ".ixx.in")
            elif filename.endswith(".hpp"):
                module_path = os.path.join(module_root, filename[:-4] + ".ixx")
            else:
                rest.append((header_path, os.path.join(module_root, filename)))
                continue

            print("Parsing file {}".format(header_path))
            sources.append(SourceCode(header_path, module_path))

    return sources, rest

def resolve_imports(modules, source_dir, macro_headers):
    modules_by_path = {}
    for module in modules:
        modules_by_path[module.header_path] = module

    for module in modules:
        includes = module.includes.copy()
        for path, export, comment in includes:
            if path in modules_by_path:
                import_module = modules_by_path[path]
                module.imports.add((import_module.module_name, import_module.fragment_name, export, comment))
                module.includes.remove((path, export, comment))

            elif export and path == os.path.join(source_dir, "hikogui", "metadata", "library_metadata.hpp"):
                module.imports.add(("hikogui.metadata.library_metadata", None, export, comment))
                module.includes.remove((path, export, comment))

            elif export:
                print("ERROR: Expect '{}' to be module to export, included from '{}'".format(path, module.header_path))
                sys.exit()

            elif path in macro_headers:
                pass

            elif not path.endswith(".hpp"):
                pass

            else:
                print("WARNING: Unknown header file '{}' included from '{}'".format(path, module.header_path))

def write_module_files(modules, source_dir):
    for module in modules:
        module.write(source_dir)

def copy_files(files_to_copy):
    for source_path, destination_path in files_to_copy:
        os.makedirs(os.path.dirname(destination_path), exist_ok=True)
        text = open(source_path, "rb").read()

        if not os.path.exists(destination_path) or open(destination_path, "rb").read() != text:
            print("Copy file '{}' to '{}'".format(source_path, destination_path))
            open(destination_path, "wb").write(text)


def main():
    source_dir = "src"
    destination_dir = "mod"
    macro_headers = [
        os.path.join(source_dir, "hikogui", "crt.hpp"),
        os.path.join(source_dir, "hikogui", "macros.hpp"),
        os.path.join(source_dir, "hikogui", "test.hpp"),
        os.path.join(source_dir, "hikogui", "win32_headers.hpp")
    ]

    if not os.path.exists(source_dir):
        print("ERROR: Expect '{}' directory in the current working directory.".format(source_dir))
        sys.exit()

    modules, rest = parse_header_files(source_dir, destination_dir, macro_headers)
    resolve_imports(modules, source_dir, macro_headers)
    
    write_module_files(modules, source_dir)

    copy_files(rest)



if __name__ == "__main__":
    main()
