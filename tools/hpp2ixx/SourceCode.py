
import os.path
import sys

class PragmaOnce (object):
    pass

class ExportModule (object):
    pass

class SourceCode (object):
    def __init__(self, header_path, module_path):
        self.header_path = header_path
        self.module_path = module_path

        self.nl = None

        self.module_name = None
        self.fragment_name = None
        self.lines = []
        # tuple(path, export)
        self.includes = set()
        # tuple(module_name, fragment_name, export)
        self.imports = set()

        self.parse(self.header_path)

    def parse(self, path):
        dirname = os.path.dirname(path)
        found_pragma_once = 0

        with open(path, "r") as fd:
            for line in fd.readlines():
                if line.startswith("#pragma once"):
                    found_pragma_once += 1
                    self.lines.append(PragmaOnce())
                    if line.endswith("\r\n"):
                        self.nl = "\r\n"
                    elif line.endswith("\n"):
                        self.nl = "\n"
                        
                elif line.startswith("hi_export_module("):
                    module_name, rest = line[17:].split(")")
                    if ":" in module_name:
                        module_name, fragment_name = module_name.split(":")
                        self.module_name = module_name.strip()
                        self.fragment_name = fragment_name.strip()
                    else:
                        self.module_name = module_name.strip()

                    self.lines.append(ExportModule())

                elif line.startswith("#include \""):
                    header_name, rest = line[10:].split("\"")
                    header_path = os.path.normpath(os.path.join(dirname, header_name))
                    self.includes.add((header_path, "export" in rest))

                else:
                    self.lines.append(line)

        if not found_pragma_once:
            print("ERROR: missing #pragma once in {}".format(path))
            sys.exit()

        if self.module_name is None:
            print("ERROR: missing hi_export_module() in {}".format(path))
            sys.exit()

    def write(self, source_path):
        module_dir = os.path.dirname(self.module_path)
        header_dir = os.path.dirname(self.header_path)

        os.makedirs(module_dir, exist_ok=True)
        with open(self.module_path, "w") as fd:
            for line in self.lines:
                if isinstance(line, PragmaOnce):
                    fd.write("module;{}".format(self.nl))

                    for include_path, export in sorted(self.includes):
                        relative_include_path = os.path.relpath(include_path, header_dir).replace("\\", "/")
                        fd.write("#include \"{}\"{}".format(relative_include_path, self.nl))

                elif isinstance(line, ExportModule):
                    module_name_str = self.module_name.replace(".", "_")
                    if self.fragment_name is not None:
                        fd.write("export module {} : {};{}".format(module_name_str, self.fragment_name, self.nl))
                    else:
                        fd.write("export module {};{}".format(module_name_str, self.nl))

                    sorted_imports = sorted(self.imports, key = lambda x: "{}:{}:{}".format("0" if x[1] is None else "1", x[0], x[1] and ""))
                    for module_name, fragment_name, export in sorted_imports:
                        export_str = "export " if export else ""
                        module_name_str = module_name.replace(".", "_")

                        if fragment_name is not None:
                            if module_name == self.module_name:
                                fd.write("{}import : {};{}".format(export_str, fragment_name, self.nl))
                            else:
                                fd.write("{}import {} : {};{}".format(export_str, module_name_str, fragment_name, self.nl))
                        else:
                            fd.write("{}import {};{}".format(export_str, module_name_str, self.nl))

                else:
                    line = line.replace("hi_export ", "export ")
                    line = line.replace("hi_inline ", "")
                    fd.write(line)
                            

