# Copyright (c) 2024-2025 Le Juez Victor
# 
# This software is provided "as-is", without any express or implied warranty. In no event 
# will the authors be held liable for any damages arising from the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose, including commercial 
# applications, and to alter it and redistribute it freely, subject to the following restrictions:
# 
#   1. The origin of this software must not be misrepresented; you must not claim that you 
#   wrote the original software. If you use this software in a product, an acknowledgment 
#   in the product documentation would be appreciated but is not required.
# 
#   2. Altered source versions must be plainly marked as such, and must not be misrepresented
#   as being the original software.
# 
#   3. This notice may not be removed or altered from any source distribution.

import sys

def file_to_c_hex(filename):
    try:
        with open(filename, "rb") as f:
            data = f.read()

        hex_array = ", ".join(f"0x{byte:02X}" for byte in data)
        print(hex_array)

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <file>")
    else:
        file_to_c_hex(sys.argv[1])
