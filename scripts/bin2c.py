import sys

def file_to_c_hex(filename):
    try:
        with open(filename, "rb") as f:
            data = f.read()

        hex_array = ", ".join(f"0x{byte:02X}" for byte in data)
        print(hex_array)

    except FileNotFoundError:
        print(f"Erreur : fichier '{filename}' introuvable.")
    except Exception as e:
        print(f"Erreur : {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <fichier>")
    else:
        file_to_c_hex(sys.argv[1])
