# logic.py
# =========================================================================
# IMPORTANT: THIS FILE NEVER SHIPS AS-IS.
# It gets encrypted by build_encrypt.py and the ciphertext is embedded
# inside checker.py before compiling checker.exe.
# The player must never see this plaintext file.
# =========================================================================

import hashlib

# Store only the SHA256 hash of the flag, never the flag itself in plaintext,
# so even after a player decrypts this file (with the correct password),
# they still have to brute-force/derive the actual flag string rather than
# just reading it off directly. Adjust REAL_FLAG below before building.

REAL_FLAG = "duck{4yBlGzSTp+5sP4Q3!eG#fE$gHGYjkpQQ}"   # <-- CHANGE THIS before building
REAL_FLAG_HASH = hashlib.sha256(REAL_FLAG.encode()).hexdigest()


def check_flag(user_input: str) -> bool:
    """Returns True if user_input matches the real flag."""
    return hashlib.sha256(user_input.strip().encode()).hexdigest() == REAL_FLAG_HASH


def get_success_message() -> str:
    return "[+] Correct! You have successfully solved the Reverse Engineering challenge."
