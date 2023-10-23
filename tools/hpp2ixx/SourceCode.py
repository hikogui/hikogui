
import os.path
import sys

class PragmaOnce (object):
    pass

class ExportModule (object):
    pass

class SystemInclude (object):
    def __init__(self, header_name):
        self.header_name = header_name

class Include (object):
    def __init__(self, header_path, header_name):
        self.header_path = header_path
        self.header_name = header_name

class ImportExport (object):
    def __init__(self, header_path):
        self.header_path = header_path

class Import (object):
    def __init__(self, module_name):
        self.module_name = module_name


class SourceCode (object):
    def __init__(self, header_path, module_path):
        self.header_path = header_path
        self.module_path = module_path

        self.module_name = None
        self.fragment_name = None
        self.lines = []
        self.system_includes = set()
        self.includes = set()
        self.export_includes = set()
        self.parse(self.header_path)

    def parse(self, path):
        dirname = os.path.dirname(path)
        lines = []
        found_pragma_once = 0

        with open(path, "r") as fd:
            for line in fd.readlines():
                if line.startswith("#pragma once"):
                    found_pragma_once += 1
                    lines.append(PragmaOnce())

                elif line.startswith("hi_export_module("):
                    self.module_name, rest = line[17:].split(")")
                    if ":" in self.module_name:
                        self.module_name, self.fragment_name = self.module_name.split(":")

                    lines.append(ExportModule())

                elif line.startswith("#include <"):
                    header_name, rest = line[10:].split(">")
                    self.system_includes.add(header_name)

                elif line.startswith("#include \""):
                    header_name, rest = line[10:].split("\"")
                    header_path = os.path.normpath(os.path.join(dirname, header_name))

                    if "export" in rest:
                        self.export_includes.add(header_path)
                    else:
                        self.includes.add(header_path)

                else:
                    lines.append(line)

        if not found_pragma_once:
            print("ERROR: missing #pragma once in {}".format(path))
            sys.exit()

        if self.module_name is None:
            print("ERROR: missing hi_export_module() in {}".format(path))
            sys.exit()

