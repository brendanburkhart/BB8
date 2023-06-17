# Embeds SPIR-V binaries into a C++ source file, so that the shaders are available at compile time
# Usage: python embed.py <output file> <shader 1 name> <shader 1 SPIR-V file> <shader 2 name> <shader 2 SPIR-V> ...

import struct
import sys

def embed_contents(name, file_path):
    with open(file_path, mode='rb') as file:
        contents = file.read()

    length = len(contents)//4
    uint32_contents = struct.unpack(f"{length}I", contents)
    initializer = ",".join(map(str,uint32_contents))

    return f"const std::vector<uint32_t> {name} = std::vector<uint32_t> {{ {initializer} }};\n"

embedded_source = """#include <vector>\n#include <cstdint>\n\n\
namespace visualization {\n\
namespace shaders {\n"""

output_file = sys.argv[1]
shaders = zip(sys.argv[2::2], sys.argv[3::2])

for name, file in shaders:
    embedded_source += embed_contents(name, file)

embedded_source += "}\n}\n"

with open(output_file, mode='w') as file:
    file.write(embedded_source)
