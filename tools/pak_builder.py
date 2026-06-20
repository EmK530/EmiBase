import os
import json
import zlib
import struct
from pathlib import Path

ASSETS_DIR = "assets"
OUTPUT_PAK = "content.epak"
MANIFEST_FILE = "tools/epak_manifest.json"

def xorshift32(state: int) -> int:
    state ^= (state << 13) & 0xFFFFFFFF
    state ^= (state >> 17) & 0xFFFFFFFF
    state ^= (state << 5) & 0xFFFFFFFF
    return state & 0xFFFFFFFF

def xor_crypt(data: bytes, start_offset: int) -> bytes:
    out = bytearray(len(data))
    seed = 0xC417A251
    state = (seed ^ start_offset) & 0xFFFFFFFF
    for _ in range(8):
        state = xorshift32(state)
    for i in range(len(data)):
        if (i & 3) == 0:
            state = xorshift32(state)
        random_byte = (state >> ((i & 3) * 8)) & 0xFF
        out[i] = data[i] ^ random_byte
    return bytes(out)

def write_encrypted(file, data: bytes):
    offset = file.tell()
    encrypted = xor_crypt(data, offset)
    file.write(encrypted)

def gather_files():
    files = []
    for root, _, filenames in os.walk(ASSETS_DIR):
        for filename in filenames:
            full_path = os.path.join(root, filename)
            virtual_path = full_path.replace("\\", "/")
            files.append(virtual_path)
    files.sort()
    return files

def build_state(files):
    state = {}
    for path in files:
        state[path] = os.path.getmtime(path)
    return state

def load_manifest():
    if not os.path.exists(MANIFEST_FILE):
        return {}
    with open(MANIFEST_FILE, "r") as f:
        return json.load(f)

def save_manifest(state):
    with open(MANIFEST_FILE, "w") as f:
        json.dump(state, f, indent=4)

files = gather_files()

current_state = build_state(files)
previous_state = load_manifest()

if current_state == previous_state and os.path.isfile(OUTPUT_PAK):
    print("[EPAK] No asset changes detected.")
    exit(0)

print("[EPAK] Changes detected, rebuilding pak...")

with open(OUTPUT_PAK, "wb") as pak:
    write_encrypted(pak, b"EPAK")
    write_encrypted(pak, struct.pack("<I", len(files)))

    for path in files:
        pathtoencode = path.replace("assets/","")
        path_bytes = pathtoencode.encode("utf-8")
        if len(path_bytes) > 255:
            raise Exception(f"Path too long (>255 bytes): {path}")

        with open(path, "rb") as f:
            data = f.read()
        write_encrypted(pak, b"EFCH")

        # Path length (1 byte)
        write_encrypted(pak, struct.pack("<B", len(path_bytes)))

        # Flags (1 byte)
        flags = 0
        write_encrypted(pak, struct.pack("<B", flags))

        # File size
        write_encrypted(pak, struct.pack("<I", len(data)))

        # CRC32 of path
        path_crc = zlib.crc32(path_bytes) & 0xFFFFFFFF
        write_encrypted(pak, struct.pack("<I", path_crc))

        # CRC32 of file data
        data_crc = zlib.crc32(data) & 0xFFFFFFFF
        write_encrypted(pak, struct.pack("<I", data_crc))

        # Path string
        write_encrypted(pak, path_bytes)

        # File data
        write_encrypted(pak, data)

        print(f"[EPAK] Packed: {path}")

save_manifest(current_state)
print(f"[EPAK] Built '{OUTPUT_PAK}' successfully.")