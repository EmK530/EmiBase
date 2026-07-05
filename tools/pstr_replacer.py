import re
import sys
import os

KEY = 0x5A # Needs to match EmiBase/include/EmiBase/ProtectedString.h

def encrypt_string(s):
    return [ord(c) ^ (KEY ^ i) for i, c in enumerate(s)]

def replace_pstr(source):
    def replacer(match):
        original = match.group(1)
        encrypted = encrypt_string(original)
        length = len(encrypted)
        bytes_literal = ','.join(f'{b}' for b in encrypted)
        return f'_pstr_decode((char[]){{{bytes_literal},0}},{length})'
    return re.sub(r'PSTR\("((?:[^"\\]|\\.)*)"\)', replacer, source)

if __name__ == '__main__':
    src_path = sys.argv[1]
    dst_path = sys.argv[2]

    if os.path.exists(dst_path) and os.path.getmtime(dst_path) >= os.path.getmtime(src_path):
        sys.exit(0)

    with open(src_path, 'r') as f:
        source = f.read()
    result = replace_pstr(source)
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)
    with open(dst_path, 'w') as f:
        f.write(result)