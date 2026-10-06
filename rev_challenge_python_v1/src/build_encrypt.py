# build_encrypt.py
# =========================================================================
# Run this ONCE, locally, as the challenge author — never ship this file
# or logic.py with the challenge.
#
# It encrypts logic.py using a key derived from PASSWORD_2 (the one
# revealed at the end of game.exe) and prints out a Python bytes literal
# you paste directly into checker.py as ENCRYPTED_LOGIC.
#
# Usage:
#   pip install cryptography
#   python build_encrypt.py
# =========================================================================

import base64
import hashlib
from cryptography.fernet import Fernet

# Must match REAL_PASSWORD_2 in game.py exactly.
PASSWORD_2 = "ch3ck3r_unl0ck_key"


def derive_key(password: str) -> bytes:
    # Fernet needs a 32-byte url-safe base64-encoded key.
    return base64.urlsafe_b64encode(hashlib.sha256(password.encode()).digest())


def main():
    key = derive_key(PASSWORD_2)
    f = Fernet(key)

    with open("logic.py", "rb") as file:
        plaintext_code = file.read()

    encrypted = f.encrypt(plaintext_code)

    print("\nCopy the line below into checker.py, replacing ENCRYPTED_LOGIC:\n")
    print(f"ENCRYPTED_LOGIC = {encrypted!r}\n")

    # Also save to a file for convenience.
    with open("encrypted_logic.txt", "w") as out:
        out.write(f"ENCRYPTED_LOGIC = {encrypted!r}\n")
    print("[+] Also saved to encrypted_logic.txt")


if __name__ == "__main__":
    main()
