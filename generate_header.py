import sys
import os

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <input_file> <output_header>")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]
array_name = os.path.splitext(os.path.basename(output_file))[0]

with open(input_file, "rb") as f:
    data = f.read()

with open(output_file, "w") as f:
    f.write(f"#pragma once\n\n")
    f.write(f"const unsigned char {array_name}[] = {{\n")

    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        f.write("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",\n")

    f.write(f"}};\n")
    f.write(f"const unsigned int {array_name}_len = {len(data)};\n")

print(f"Generated {output_file} ({len(data)} bytes)")
