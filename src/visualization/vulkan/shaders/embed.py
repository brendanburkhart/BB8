# Embeds SPIR-V binaries into a C++ source file, so that the shaders are available at compile time
# Usage: python embed.py <output file> <shader 1 name> <shader 1 SPIR-V file> <shader 2 name> <shader 2 SPIR-V> ...

import os
import struct
import sys

def embed_contents(name, file_path):
    with open(file_path, mode='rb') as file:
        contents = file.read()

    length = len(contents)//4
    uint32_contents = struct.unpack(f"{length}I", contents)
    initializer = ",".join(map(str,uint32_contents))

    return f"const std::vector<uint32_t> {name} = std::vector<uint32_t> {{ {initializer} }};\n\n"


output_file_path = sys.argv[1]
shaders = zip(sys.argv[2::2], sys.argv[3::2])

output_file = os.path.basename(output_file_path)
include_guard_name = output_file.replace(".", "_").upper()
embedded_source = """\
#ifndef BB8_VISUALIZATION_SHADERS_{}\n\
#define BB8_VISUALIZATION_SHADERS_{}\n\n\
#include <vector>\n#include <cstdint>\n\n\
namespace visualization {{\n\
namespace vulkan {{\n\
namespace shaders {{\n\n""".format(include_guard_name, include_guard_name)

for name, file in shaders:
    embedded_source += embed_contents(name, file)

embedded_source += "}\n}\n}\n#endif"

with open(output_file_path, mode='w') as file:
    file.write(embedded_source)
