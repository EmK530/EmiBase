import os
import json
import zlib
import struct
import hashlib
from pathlib import Path

ASSETS_DIR = "assets"
OUTPUT_PAK = "content.epak"
MANIFEST_FILE = "tools/epak_manifest.json"
VERSION = 2

ENCRYPT_DATA = False # Should the pak file be encrypted?
ANTI_CORRUPTION = False # Should ContentManager detect corrupted assets?

def file_hash(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        h.update(f.read())
    return h.hexdigest()

BUILDER_HASH = file_hash(__file__)

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

def write(file, data: bytes, no_encrypt=False):
    if ENCRYPT_DATA and not no_encrypt:
        offset = file.tell()
        encrypted = xor_crypt(data, offset)
        file.write(encrypted)
    else:
        file.write(data)

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
    state = {"source": BUILDER_HASH, "files": {}}
    for path in files:
        state["files"][path] = os.path.getmtime(path)
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
    write(pak, b"EPAK", True)
    write(pak, struct.pack("<B", VERSION), True)

    flags = 0
    if ENCRYPT_DATA:
        flags += 1
    if ANTI_CORRUPTION:
        flags += 2
    write(pak, struct.pack("<B", flags), True)

    write(pak, struct.pack("<I", len(files)))

    for path in files:
        pathtoencode = path.replace("assets/","")
        path_bytes = pathtoencode.encode("utf-8")
        if len(path_bytes) > 255:
            raise Exception(f"Path too long (>255 bytes): {path}")

        with open(path, "rb") as f:
            data = f.read()
        write(pak, b"EFCH")

        # Path length (1 byte)
        write(pak, struct.pack("<B", len(path_bytes)))

        # File size
        write(pak, struct.pack("<I", len(data)))

        if ANTI_CORRUPTION:
            # CRC32 of path
            path_crc = zlib.crc32(path_bytes) & 0xFFFFFFFF
            write(pak, struct.pack("<I", path_crc))

            # CRC32 of file data
            data_crc = zlib.crc32(data) & 0xFFFFFFFF
            write(pak, struct.pack("<I", data_crc))

        # Path string
        write(pak, path_bytes)

        # File data
        write(pak, data)

        print(f"[EPAK] Packed: {path}")

save_manifest(current_state)
print(f"[EPAK] Built '{OUTPUT_PAK}' successfully.")