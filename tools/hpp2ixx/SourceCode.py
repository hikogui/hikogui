
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
                    rest = rest.strip()

                    export = False
                    comment = None
                    if rest.startswith("//"):
                        rest = rest[2:].strip()
                        if rest.startswith("export"):
                            export = True
                            rest = rest[6:].strip()

                        if rest:
                            comment = rest

                    header_path = os.path.normpath(os.path.join(dirname, header_name))
                    self.includes.add((header_path, export, comment))

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

        text = []
        for line in self.lines:
            if isinstance(line, PragmaOnce):
                text.append("module;{}".format(self.nl))

                for include_path, export, comment in sorted(self.includes):
                    relative_include_path = os.path.relpath(include_path, header_dir).replace("\\", "/")
                    comment_str = " // {}".format(comment) if comment else ""
                    text.append("#include \"{}\"{}{}".format(relative_include_path, comment_str, self.nl))

            elif isinstance(line, ExportModule):
                module_name_str = self.module_name.replace(".", "_")
                if self.fragment_name is not None:
                    text.append("export module {} : {};{}".format(module_name_str, self.fragment_name, self.nl))
                else:
                    text.append("export module {};{}".format(module_name_str, self.nl))

                sorted_imports = list(self.imports)
                sorted_imports.sort(key = lambda x: (x[1] is None, x[0], x[1] or ""))
                for module_name, fragment_name, export, comment in sorted_imports:
                    export_str = "export " if export else ""
                    comment_str = " // {}".format(comment) if comment else ""
                    module_name_str = module_name.replace(".", "_")

                    if fragment_name is not None:
                        if module_name == self.module_name:
                            text.append("{}import : {};{}{}".format(export_str, fragment_name, comment_str, self.nl))
                        else:
                            text.append("{}import {} : {};{}{}".format(export_str, module_name_str, fragment_name, comment_str, self.nl))
                    else:
                        text.append("{}import {};{}{}".format(export_str, module_name_str, comment_str, self.nl))

            else:
                line = line.replace("hi_export ", "export ")
                line = line.replace("hi_inline ", "")
                text.append(line)

        text = "".join(text)

        os.makedirs(module_dir, exist_ok=True)
        if not os.path.exists(self.module_path) or open(self.module_path, "r").read() != text:
            print("Writing file {}".format(self.module_path))
            open(self.module_path, "w").write(text)
